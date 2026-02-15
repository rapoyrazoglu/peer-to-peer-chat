#include "peerchat/client.hpp"

#include "peerchat/connection.hpp"

#include <spdlog/spdlog.h>

namespace peerchat {

Client::Client(asio::io_context& io, const std::string& host, uint16_t port,
               ConnectCallback on_connect, ErrorCallback on_error)
    : socket_(io),
      on_connect_(std::move(on_connect)),
      on_error_(std::move(on_error)) {
    asio::ip::tcp::resolver resolver(io);
    auto endpoints = resolver.resolve(host, std::to_string(port));
    do_connect(endpoints);
}

void Client::do_connect(asio::ip::tcp::resolver::results_type endpoints) {
    asio::async_connect(
        socket_, endpoints,
        [this](asio::error_code ec, const asio::ip::tcp::endpoint& ep) {
            if (ec) {
                spdlog::error("Connect failed: {}", ec.message());
                if (on_error_) {
                    on_error_(ec.message());
                }
                return;
            }

            spdlog::info("Connected to {}:{}", ep.address().to_string(),
                         ep.port());
            auto conn = Connection::create(std::move(socket_));
            if (on_connect_) {
                on_connect_(conn);
            }
        });
}

} // namespace peerchat
