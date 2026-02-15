#include "peerchat/app.hpp"

#include "peerchat/client.hpp"

#include <spdlog/spdlog.h>

namespace peerchat {

App::App(uint16_t port, const std::string& nickname)
    : identity_(nickname), peer_manager_(io_, identity_) {
    // Set up server
    server_ = std::make_unique<Server>(
        io_, port, [this](ConnectionPtr conn) {
            if (peer_manager_.state() != PeerState::Disconnected) {
                cli_.display_system(
                    "Rejected connection: already connected to a peer.");
                conn->close();
                return;
            }
            cli_.display_system("Incoming connection from " +
                                conn->remote_address());
            peer_manager_.set_connection(conn, false);
        });

    // Wire peer_manager callbacks
    peer_manager_.on_display(
        [this](const std::string& nick, const std::string& body) {
            cli_.display_message(nick, body);
        });

    peer_manager_.on_ack(
        [this](const std::string& msg_id) { cli_.display_ack(msg_id); });

    peer_manager_.on_state_change([this](PeerState state) {
        if (state == PeerState::Connected) {
            cli_.display_system("Connected to " +
                                peer_manager_.remote_nickname() + " (" +
                                peer_manager_.remote_address() + ")");
        }
    });

    peer_manager_.on_disconnect([this](const std::string& reason) {
        cli_.display_system("Disconnected: " + reason);
    });

    // Wire CLI commands
    cli_.on_connect_command(
        [this](const std::string& host, uint16_t port) {
            connect_to(host, port);
        });

    cli_.on_disconnect_command([this]() {
        peer_manager_.disconnect();
        cli_.display_system("Disconnected.");
    });

    cli_.on_status_command([this]() { show_status(); });

    cli_.on_quit_command([this]() { shutdown(); });

    cli_.on_text_input([this](const std::string& text) {
        if (peer_manager_.state() != PeerState::Connected) {
            cli_.display_system("Not connected. Use /connect <host>:<port>");
            return;
        }
        peer_manager_.send_text(text);
    });
}

App::~App() { shutdown(); }

void App::run() {
    cli_.display_system("PeerChat v0.1.0 | Nickname: " +
                        identity_.nickname() + " | Port: " +
                        std::to_string(server_->port()));

    // Run io_context in background thread
    auto work = asio::make_work_guard(io_);
    io_thread_ = std::thread([this, work = std::move(work)]() mutable {
        io_.run();
    });

    // CLI runs on main thread (blocking)
    cli_.run();

    shutdown();
}

void App::connect_to(const std::string& host, uint16_t port) {
    if (peer_manager_.state() != PeerState::Disconnected) {
        cli_.display_system("Already connected. /disconnect first.");
        return;
    }

    cli_.display_system("Connecting to " + host + ":" +
                        std::to_string(port) + "...");

    // Client must be kept alive; allocate on heap, capture shared_ptr
    asio::post(io_, [this, host, port]() {
        // Client self-destructs after connect callback fires
        new Client(
            io_, host, port,
            [this](ConnectionPtr conn) {
                peer_manager_.set_connection(conn, true);
            },
            [this](const std::string& err) {
                cli_.display_system("Connection failed: " + err);
            });
    });
}

void App::show_status() {
    cli_.display_system("State: " +
                        peer_state_to_string(peer_manager_.state()));
    cli_.display_system("Peer ID: " + identity_.peer_id());
    cli_.display_system("Nickname: " + identity_.nickname());
    if (peer_manager_.state() == PeerState::Connected) {
        cli_.display_system("Remote: " + peer_manager_.remote_nickname() +
                            " (" + peer_manager_.remote_peer_id() + ")");
        cli_.display_system("Address: " + peer_manager_.remote_address());
    }
}

void App::shutdown() {
    peer_manager_.disconnect();
    if (server_) {
        server_->stop();
    }
    io_.stop();
    if (io_thread_.joinable()) {
        io_thread_.join();
    }
}

} // namespace peerchat
