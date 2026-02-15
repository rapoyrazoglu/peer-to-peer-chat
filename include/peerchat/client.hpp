#pragma once

#include "peerchat/types.hpp"

#include <asio.hpp>
#include <string>

namespace peerchat {

class Client {
  public:
    Client(asio::io_context& io, const std::string& host, uint16_t port,
           ConnectCallback on_connect, ErrorCallback on_error);

  private:
    void do_connect(asio::ip::tcp::resolver::results_type endpoints);

    asio::ip::tcp::socket socket_;
    ConnectCallback on_connect_;
    ErrorCallback on_error_;
};

} // namespace peerchat
