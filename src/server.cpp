#include "peerchat/server.hpp"

#include "peerchat/connection.hpp"

#include <spdlog/spdlog.h>

namespace peerchat {

Server::Server(asio::io_context& io, uint16_t port, ConnectCallback on_connect)
    : acceptor_(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
      on_connect_(std::move(on_connect)) {
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    spdlog::info("Listening on port {}", this->port());
    do_accept();
}

void Server::stop() {
    asio::error_code ec;
    acceptor_.close(ec);
}

uint16_t Server::port() const {
    return acceptor_.local_endpoint().port();
}

void Server::do_accept() {
    acceptor_.async_accept(
        [this](asio::error_code ec, asio::ip::tcp::socket socket) {
            if (ec) {
                if (ec != asio::error::operation_aborted) {
                    spdlog::error("Accept error: {}", ec.message());
                }
                return;
            }

            spdlog::info("Accepted connection from {}:{}",
                         socket.remote_endpoint().address().to_string(),
                         socket.remote_endpoint().port());

            auto conn = Connection::create(std::move(socket));
            if (on_connect_) {
                on_connect_(conn);
            }

            do_accept();
        });
}

} // namespace peerchat
