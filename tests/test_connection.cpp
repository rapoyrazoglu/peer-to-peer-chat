#include "peerchat/connection.hpp"
#include "peerchat/message.hpp"
#include "peerchat/server.hpp"

#include <asio.hpp>
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>

using namespace peerchat;

class ConnectionTest : public ::testing::Test {
  protected:
    void SetUp() override { io_ = std::make_unique<asio::io_context>(); }

    void TearDown() override {
        io_->stop();
        if (io_thread_.joinable()) io_thread_.join();
    }

    void run_io() {
        auto work = asio::make_work_guard(*io_);
        io_thread_ = std::thread([this, work = std::move(work)]() mutable {
            io_->run();
        });
    }

    std::unique_ptr<asio::io_context> io_;
    std::thread io_thread_;
};

TEST_F(ConnectionTest, LoopbackSendReceive) {
    std::atomic<bool> received{false};
    std::string received_body;
    std::atomic<bool> server_error{false};
    ConnectionPtr server_conn;

    Server server(*io_, 0, [&](ConnectionPtr conn) {
        server_conn = conn;
        conn->start(
            [&](const std::string& json) {
                auto msg = Message::deserialize(json);
                received_body = msg.body;
                received.store(true);
            },
            [&](const std::string&) { server_error.store(true); });
    });

    auto port = server.port();
    run_io();

    // Connect using a raw socket to avoid Client class lifecycle issues
    std::atomic<bool> connected{false};
    ConnectionPtr client_conn;

    asio::post(*io_, [&]() {
        auto socket =
            std::make_shared<asio::ip::tcp::socket>(*io_);
        socket->async_connect(
            asio::ip::tcp::endpoint(
                asio::ip::address::from_string("127.0.0.1"), port),
            [&, socket](asio::error_code ec) {
                ASSERT_FALSE(ec) << ec.message();
                client_conn = Connection::create(std::move(*socket));
                client_conn->start([](const std::string&) {},
                                   [](const std::string&) {});
                connected.store(true);
            });
    });

    // Wait for connection
    for (int i = 0; i < 100 && !connected.load(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ASSERT_TRUE(connected.load());
    ASSERT_FALSE(server_error.load());

    // Send a message from client to server
    auto msg = Message::make_text("peer-1", "alice", "0000", "Hello from client!");
    client_conn->send(msg.serialize());

    // Wait for receipt
    for (int i = 0; i < 100 && !received.load(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_TRUE(received.load());
    EXPECT_EQ(received_body, "Hello from client!");

    // Clean up
    if (client_conn) client_conn->close();
    if (server_conn) server_conn->close();
    server.stop();
}

TEST_F(ConnectionTest, BidirectionalCommunication) {
    std::atomic<int> server_count{0};
    std::atomic<int> client_count{0};
    ConnectionPtr server_conn;
    ConnectionPtr client_conn;

    Server server(*io_, 0, [&](ConnectionPtr conn) {
        server_conn = conn;
        conn->start(
            [&](const std::string& json) {
                server_count.fetch_add(1);
                server_conn->send(json);
            },
            [](const std::string&) {});
    });

    auto port = server.port();
    run_io();

    std::atomic<bool> connected{false};

    asio::post(*io_, [&]() {
        auto socket =
            std::make_shared<asio::ip::tcp::socket>(*io_);
        socket->async_connect(
            asio::ip::tcp::endpoint(
                asio::ip::address::from_string("127.0.0.1"), port),
            [&, socket](asio::error_code ec) {
                ASSERT_FALSE(ec) << ec.message();
                client_conn = Connection::create(std::move(*socket));
                client_conn->start(
                    [&](const std::string&) {
                        client_count.fetch_add(1);
                    },
                    [](const std::string&) {});
                connected.store(true);
            });
    });

    for (int i = 0; i < 100 && !connected.load(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ASSERT_TRUE(connected.load());

    // Send 3 messages
    for (int i = 0; i < 3; ++i) {
        auto msg = Message::make_text("peer-1", "alice", "0000",
                                      "msg-" + std::to_string(i));
        client_conn->send(msg.serialize());
    }

    // Wait for all echoes
    for (int i = 0; i < 200 && client_count.load() < 3; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_EQ(server_count.load(), 3);
    EXPECT_EQ(client_count.load(), 3);

    if (client_conn) client_conn->close();
    if (server_conn) server_conn->close();
    server.stop();
}
