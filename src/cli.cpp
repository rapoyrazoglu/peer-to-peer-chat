#include "peerchat/cli.hpp"

#include <iostream>
#include <sstream>

namespace peerchat {

Cli::Cli() {}

void Cli::run() {
    display_system("Type /help for commands, or just type to chat.");
    std::string line;
    while (running_ && std::getline(std::cin, line)) {
        if (line.empty()) continue;
        process_line(line);
    }
}

void Cli::process_line(const std::string& line) {
    if (line[0] != '/') {
        if (on_text_) on_text_(line);
        return;
    }

    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd == "/connect") {
        std::string addr;
        iss >> addr;
        if (addr.empty()) {
            display_system("Usage: /connect <host>:<port>");
            return;
        }

        auto colon = addr.rfind(':');
        if (colon == std::string::npos) {
            display_system("Usage: /connect <host>:<port>");
            return;
        }

        std::string host = addr.substr(0, colon);
        uint16_t port;
        try {
            port = static_cast<uint16_t>(std::stoi(addr.substr(colon + 1)));
        } catch (...) {
            display_system("Invalid port number");
            return;
        }

        if (on_connect_) on_connect_(host, port);
    } else if (cmd == "/disconnect") {
        if (on_disconnect_) on_disconnect_();
    } else if (cmd == "/status") {
        if (on_status_) on_status_();
    } else if (cmd == "/quit" || cmd == "/exit") {
        running_ = false;
        if (on_quit_) on_quit_();
    } else if (cmd == "/help") {
        display_system("Commands:");
        display_system("  /connect <host>:<port>  - Connect to a peer");
        display_system("  /disconnect             - Disconnect from peer");
        display_system("  /status                 - Show connection status");
        display_system("  /quit                   - Exit PeerChat");
    } else {
        display_system("Unknown command: " + cmd + " (type /help)");
    }
}

void Cli::display_message(const std::string& nick, const std::string& body) {
    print("[" + nick + "] " + body);
}

void Cli::display_system(const std::string& msg) { print("* " + msg); }

void Cli::display_ack(const std::string& msg_id) {
    print("  (delivered: " + msg_id.substr(0, 8) + "...)");
}

void Cli::print(const std::string& text) {
    std::lock_guard lock(output_mutex_);
    std::cout << text << std::endl;
}

} // namespace peerchat
