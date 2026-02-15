#pragma once

#include "peerchat/cli.hpp"
#include "peerchat/identity.hpp"
#include "peerchat/peer_manager.hpp"
#include "peerchat/server.hpp"

#include <asio.hpp>
#include <memory>
#include <thread>

namespace peerchat {

class App {
  public:
    App(uint16_t port, const std::string& nickname);
    ~App();

    void run();

  private:
    void connect_to(const std::string& host, uint16_t port);
    void show_status();
    void shutdown();

    asio::io_context io_;
    Identity identity_;
    std::unique_ptr<Server> server_;
    PeerManager peer_manager_;
    Cli cli_;

    std::thread io_thread_;
};

} // namespace peerchat
