#pragma once

#include "peerchat/framing.hpp"
#include "peerchat/types.hpp"

#include <asio.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

namespace peerchat {

class Connection : public std::enable_shared_from_this<Connection> {
  public:
    static ConnectionPtr create(asio::ip::tcp::socket socket);

    void start(MessageCallback on_message, ErrorCallback on_error);
    void send(const std::string& json);
    void close();

    std::string remote_address() const;
    bool is_open() const;

  private:
    explicit Connection(asio::ip::tcp::socket socket);

    void do_read();
    void do_write();

    asio::ip::tcp::socket socket_;
    FrameDecoder decoder_;
    MessageCallback on_message_;
    ErrorCallback on_error_;

    std::array<uint8_t, 4096> read_buf_;

    std::mutex write_mutex_;
    std::queue<std::vector<uint8_t>> write_queue_;
    bool writing_{false};
};

} // namespace peerchat
