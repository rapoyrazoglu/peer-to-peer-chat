#pragma once

#include "peerchat/types.hpp"

#include <asio.hpp>
#include <cstdint>

namespace peerchat {

class Server {
  public:
    Server(asio::io_context& io, uint16_t port, ConnectCallback on_connect);
    void stop();
    uint16_t port() const;

  private:
    void do_accept();

    asio::ip::tcp::acceptor acceptor_;
    ConnectCallback on_connect_;
};

} // namespace peerchat
