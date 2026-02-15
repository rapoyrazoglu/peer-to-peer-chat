#pragma once

#include "peerchat/connection.hpp"
#include "peerchat/identity.hpp"
#include "peerchat/message.hpp"
#include "peerchat/types.hpp"

#include <asio.hpp>
#include <functional>
#include <memory>
#include <string>

namespace peerchat {

enum class PeerState {
    Disconnected,
    Connecting,
    WaitingHandshake,
    Connected,
};

std::string peer_state_to_string(PeerState state);

using DisplayCallback =
    std::function<void(const std::string& nick, const std::string& body)>;
using AckCallback = std::function<void(const std::string& msg_id)>;
using StateChangeCallback = std::function<void(PeerState state)>;

class PeerManager {
  public:
    PeerManager(asio::io_context& io, Identity& identity);

    // Set up a new connection (inbound or outbound).
    // is_initiator: true if we initiated the connection (send handshake first).
    void set_connection(ConnectionPtr conn, bool is_initiator);

    void send_text(const std::string& body);
    void disconnect();

    PeerState state() const { return state_; }
    std::string remote_nickname() const { return remote_nickname_; }
    std::string remote_tag() const { return remote_tag_; }
    std::string remote_display_name() const {
        return remote_nickname_ + "#" + remote_tag_;
    }
    std::string remote_peer_id() const { return remote_peer_id_; }
    std::string remote_address() const;

    void on_display(DisplayCallback cb) { on_display_ = std::move(cb); }
    void on_ack(AckCallback cb) { on_ack_ = std::move(cb); }
    void on_state_change(StateChangeCallback cb) {
        on_state_change_ = std::move(cb);
    }
    void on_disconnect(DisconnectCallback cb) {
        on_disconnect_ = std::move(cb);
    }

  private:
    void handle_message(const std::string& json);
    void handle_error(const std::string& reason);

    void handle_handshake(const Message& msg);
    void handle_text(const Message& msg);
    void handle_ack(const Message& msg);
    void handle_ping(const Message& msg);
    void handle_pong();

    void send_handshake();
    void start_handshake_timer();
    void start_heartbeat();
    void reset_pong_timer();

    void set_state(PeerState new_state);
    void cleanup();

    asio::io_context& io_;
    Identity& identity_;
    ConnectionPtr conn_;
    PeerState state_{PeerState::Disconnected};
    bool is_initiator_{false};

    std::string remote_nickname_;
    std::string remote_tag_;
    std::string remote_peer_id_;

    asio::steady_timer handshake_timer_;
    asio::steady_timer ping_timer_;
    asio::steady_timer pong_timer_;

    DisplayCallback on_display_;
    AckCallback on_ack_;
    StateChangeCallback on_state_change_;
    DisconnectCallback on_disconnect_;

    static constexpr int kHandshakeTimeoutSec = 5;
    static constexpr int kPingIntervalSec = 30;
    static constexpr int kPongTimeoutSec = 10;
};

} // namespace peerchat
