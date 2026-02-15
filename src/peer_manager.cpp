#include "peerchat/peer_manager.hpp"

#include <spdlog/spdlog.h>

namespace peerchat {

std::string peer_state_to_string(PeerState state) {
    switch (state) {
        case PeerState::Disconnected: return "disconnected";
        case PeerState::Connecting: return "connecting";
        case PeerState::WaitingHandshake: return "waiting_handshake";
        case PeerState::Connected: return "connected";
    }
    return "unknown";
}

PeerManager::PeerManager(asio::io_context& io, Identity& identity)
    : io_(io),
      identity_(identity),
      handshake_timer_(io),
      ping_timer_(io),
      pong_timer_(io) {}

void PeerManager::set_connection(ConnectionPtr conn, bool is_initiator) {
    if (state_ != PeerState::Disconnected) {
        cleanup();
    }

    conn_ = std::move(conn);
    is_initiator_ = is_initiator;
    set_state(PeerState::WaitingHandshake);

    conn_->start(
        [this](const std::string& json) { handle_message(json); },
        [this](const std::string& reason) { handle_error(reason); });

    if (is_initiator_) {
        send_handshake();
    }

    start_handshake_timer();
}

void PeerManager::send_text(const std::string& body) {
    if (state_ != PeerState::Connected || !conn_) {
        spdlog::warn("Cannot send: not connected");
        return;
    }

    auto msg =
        Message::make_text(identity_.peer_id(), identity_.nickname(), body);
    conn_->send(msg.serialize());
    spdlog::debug("Sent text [{}]: {}", msg.id, body);
}

void PeerManager::disconnect() { cleanup(); }

std::string PeerManager::remote_address() const {
    if (conn_) return conn_->remote_address();
    return "<not connected>";
}

void PeerManager::handle_message(const std::string& json) {
    try {
        auto msg = Message::deserialize(json);
        switch (msg.type) {
            case MessageType::Handshake: handle_handshake(msg); break;
            case MessageType::Text: handle_text(msg); break;
            case MessageType::Ack: handle_ack(msg); break;
            case MessageType::Ping: handle_ping(msg); break;
            case MessageType::Pong: handle_pong(); break;
        }
    } catch (const std::exception& e) {
        spdlog::error("Failed to parse message: {}", e.what());
    }
}

void PeerManager::handle_error(const std::string& reason) {
    spdlog::info("Connection error: {}", reason);
    auto old_state = state_;
    cleanup();
    if (old_state != PeerState::Disconnected && on_disconnect_) {
        on_disconnect_(reason);
    }
}

void PeerManager::handle_handshake(const Message& msg) {
    if (state_ != PeerState::WaitingHandshake) {
        spdlog::warn("Unexpected handshake in state {}",
                     peer_state_to_string(state_));
        return;
    }

    remote_peer_id_ = msg.sender;
    remote_nickname_ = msg.nickname;
    spdlog::info("Handshake from {} ({})", remote_nickname_, remote_peer_id_);

    handshake_timer_.cancel();

    // If we're the acceptor, send our handshake back
    if (!is_initiator_) {
        send_handshake();
    }

    set_state(PeerState::Connected);
    start_heartbeat();
}

void PeerManager::handle_text(const Message& msg) {
    if (state_ != PeerState::Connected) return;

    spdlog::debug("Received text [{}] from {}: {}", msg.id, msg.nickname,
                  msg.body);

    // Send ACK
    auto ack = Message::make_ack(identity_.peer_id(), msg.id);
    conn_->send(ack.serialize());

    if (on_display_) {
        on_display_(msg.nickname, msg.body);
    }
}

void PeerManager::handle_ack(const Message& msg) {
    if (state_ != PeerState::Connected) return;
    spdlog::debug("ACK received for message {}", msg.id);
    if (on_ack_) {
        on_ack_(msg.id);
    }
}

void PeerManager::handle_ping(const Message& msg) {
    if (state_ != PeerState::Connected || !conn_) return;
    auto pong = Message::make_pong(identity_.peer_id());
    conn_->send(pong.serialize());
    spdlog::debug("Responded to ping from {}", msg.sender);
}

void PeerManager::handle_pong() {
    if (state_ != PeerState::Connected) return;
    spdlog::debug("Pong received");
    pong_timer_.cancel();
}

void PeerManager::send_handshake() {
    if (!conn_) return;
    auto msg =
        Message::make_handshake(identity_.peer_id(), identity_.nickname());
    conn_->send(msg.serialize());
    spdlog::debug("Sent handshake");
}

void PeerManager::start_handshake_timer() {
    handshake_timer_.expires_after(
        std::chrono::seconds(kHandshakeTimeoutSec));
    handshake_timer_.async_wait([this](asio::error_code ec) {
        if (ec) return;
        if (state_ == PeerState::WaitingHandshake) {
            spdlog::warn("Handshake timeout");
            handle_error("handshake timeout");
        }
    });
}

void PeerManager::start_heartbeat() {
    ping_timer_.expires_after(std::chrono::seconds(kPingIntervalSec));
    ping_timer_.async_wait([this](asio::error_code ec) {
        if (ec) return;
        if (state_ != PeerState::Connected || !conn_) return;

        auto ping = Message::make_ping(identity_.peer_id());
        conn_->send(ping.serialize());
        spdlog::debug("Sent ping");

        reset_pong_timer();
        start_heartbeat();
    });
}

void PeerManager::reset_pong_timer() {
    pong_timer_.expires_after(std::chrono::seconds(kPongTimeoutSec));
    pong_timer_.async_wait([this](asio::error_code ec) {
        if (ec) return;
        if (state_ == PeerState::Connected) {
            spdlog::warn("Pong timeout — disconnecting");
            handle_error("heartbeat timeout");
        }
    });
}

void PeerManager::set_state(PeerState new_state) {
    if (state_ == new_state) return;
    spdlog::info("State: {} -> {}", peer_state_to_string(state_),
                 peer_state_to_string(new_state));
    state_ = new_state;
    if (on_state_change_) {
        on_state_change_(new_state);
    }
}

void PeerManager::cleanup() {
    handshake_timer_.cancel();
    ping_timer_.cancel();
    pong_timer_.cancel();

    if (conn_) {
        conn_->close();
        conn_.reset();
    }

    remote_nickname_.clear();
    remote_peer_id_.clear();
    set_state(PeerState::Disconnected);
}

} // namespace peerchat
