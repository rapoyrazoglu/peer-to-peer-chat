#include "peerchat/connection.hpp"

#include <spdlog/spdlog.h>

namespace peerchat {

ConnectionPtr Connection::create(asio::ip::tcp::socket socket) {
    return ConnectionPtr(new Connection(std::move(socket)));
}

Connection::Connection(asio::ip::tcp::socket socket)
    : socket_(std::move(socket)) {}

void Connection::start(MessageCallback on_message, ErrorCallback on_error) {
    on_message_ = std::move(on_message);
    on_error_ = std::move(on_error);
    do_read();
}

void Connection::send(const std::string& json) {
    auto frame = FrameEncoder::encode(json);

    bool should_write = false;
    {
        std::lock_guard lock(write_mutex_);
        write_queue_.push(std::move(frame));
        if (!writing_) {
            writing_ = true;
            should_write = true;
        }
    }
    if (should_write) {
        do_write();
    }
}

void Connection::close() {
    asio::error_code ec;
    socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
    socket_.close(ec);
}

std::string Connection::remote_address() const {
    try {
        auto ep = socket_.remote_endpoint();
        return ep.address().to_string() + ":" + std::to_string(ep.port());
    } catch (...) {
        return "<unknown>";
    }
}

bool Connection::is_open() const { return socket_.is_open(); }

void Connection::do_read() {
    auto self = shared_from_this();
    socket_.async_read_some(
        asio::buffer(read_buf_),
        [this, self](asio::error_code ec, std::size_t bytes_read) {
            if (ec) {
                if (ec != asio::error::operation_aborted && on_error_) {
                    on_error_(ec.message());
                }
                return;
            }

            decoder_.feed(read_buf_.data(), bytes_read);
            while (auto frame = decoder_.next()) {
                if (on_message_) {
                    on_message_(*frame);
                }
            }

            do_read();
        });
}

void Connection::do_write() {
    auto self = shared_from_this();
    // Lock briefly to get front of queue
    std::vector<uint8_t>* front;
    {
        std::lock_guard lock(write_mutex_);
        if (write_queue_.empty()) {
            writing_ = false;
            return;
        }
        front = &write_queue_.front();
    }

    asio::async_write(
        socket_, asio::buffer(*front),
        [this, self](asio::error_code ec, std::size_t /*bytes_written*/) {
            if (ec) {
                if (ec != asio::error::operation_aborted && on_error_) {
                    on_error_(ec.message());
                }
                return;
            }

            {
                std::lock_guard lock(write_mutex_);
                write_queue_.pop();
            }
            do_write();
        });
}

} // namespace peerchat
