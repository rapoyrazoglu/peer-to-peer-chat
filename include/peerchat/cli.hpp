#pragma once

#include <functional>
#include <mutex>
#include <string>

namespace peerchat {

using ConnectCommandCallback =
    std::function<void(const std::string& host, uint16_t port)>;
using SimpleCallback = std::function<void()>;
using TextInputCallback = std::function<void(const std::string& text)>;

class Cli {
  public:
    Cli();

    // Blocking: reads from stdin until /quit or EOF
    void run();

    void on_connect_command(ConnectCommandCallback cb) {
        on_connect_ = std::move(cb);
    }
    void on_disconnect_command(SimpleCallback cb) {
        on_disconnect_ = std::move(cb);
    }
    void on_status_command(SimpleCallback cb) { on_status_ = std::move(cb); }
    void on_quit_command(SimpleCallback cb) { on_quit_ = std::move(cb); }
    void on_text_input(TextInputCallback cb) { on_text_ = std::move(cb); }

    // Thread-safe display methods
    void display_message(const std::string& nick, const std::string& body);
    void display_system(const std::string& msg);
    void display_ack(const std::string& msg_id);

  private:
    void process_line(const std::string& line);
    void print(const std::string& text);

    ConnectCommandCallback on_connect_;
    SimpleCallback on_disconnect_;
    SimpleCallback on_status_;
    SimpleCallback on_quit_;
    TextInputCallback on_text_;

    std::mutex output_mutex_;
    bool running_{true};
};

} // namespace peerchat
