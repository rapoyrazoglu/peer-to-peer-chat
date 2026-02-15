#include "peerchat/message.hpp"

#include <gtest/gtest.h>

using namespace peerchat;

TEST(MessageTest, TypeStringRoundtrip) {
    EXPECT_EQ(message_type_to_string(MessageType::Handshake), "handshake");
    EXPECT_EQ(message_type_from_string("handshake"), MessageType::Handshake);
    EXPECT_EQ(message_type_to_string(MessageType::Text), "text");
    EXPECT_EQ(message_type_from_string("text"), MessageType::Text);
    EXPECT_EQ(message_type_to_string(MessageType::Ack), "ack");
    EXPECT_EQ(message_type_from_string("ack"), MessageType::Ack);
    EXPECT_EQ(message_type_to_string(MessageType::Ping), "ping");
    EXPECT_EQ(message_type_from_string("ping"), MessageType::Ping);
    EXPECT_EQ(message_type_to_string(MessageType::Pong), "pong");
    EXPECT_EQ(message_type_from_string("pong"), MessageType::Pong);
}

TEST(MessageTest, InvalidTypeThrows) {
    EXPECT_THROW(message_type_from_string("bogus"), std::invalid_argument);
}

TEST(MessageTest, HandshakeSerializationRoundtrip) {
    auto msg = Message::make_handshake("peer-123", "alice", "1530");
    auto json_str = msg.serialize();
    auto restored = Message::deserialize(json_str);

    EXPECT_EQ(restored.type, MessageType::Handshake);
    EXPECT_EQ(restored.sender, "peer-123");
    EXPECT_EQ(restored.nickname, "alice");
    EXPECT_EQ(restored.tag, "1530");
    EXPECT_EQ(restored.id, msg.id);
    EXPECT_EQ(restored.timestamp, msg.timestamp);
}

TEST(MessageTest, TextSerializationRoundtrip) {
    auto msg = Message::make_text("peer-456", "bob", "2847", "Hello, world!");
    auto json_str = msg.serialize();
    auto restored = Message::deserialize(json_str);

    EXPECT_EQ(restored.type, MessageType::Text);
    EXPECT_EQ(restored.sender, "peer-456");
    EXPECT_EQ(restored.nickname, "bob");
    EXPECT_EQ(restored.tag, "2847");
    EXPECT_EQ(restored.body, "Hello, world!");
}

TEST(MessageTest, AckSerializationRoundtrip) {
    auto msg = Message::make_ack("peer-789", "original-msg-id");
    auto json_str = msg.serialize();
    auto restored = Message::deserialize(json_str);

    EXPECT_EQ(restored.type, MessageType::Ack);
    EXPECT_EQ(restored.id, "original-msg-id");
    EXPECT_EQ(restored.sender, "peer-789");
}

TEST(MessageTest, PingPongRoundtrip) {
    auto ping = Message::make_ping("peer-aaa");
    auto pong = Message::make_pong("peer-bbb");

    auto ping_restored = Message::deserialize(ping.serialize());
    auto pong_restored = Message::deserialize(pong.serialize());

    EXPECT_EQ(ping_restored.type, MessageType::Ping);
    EXPECT_EQ(pong_restored.type, MessageType::Pong);
}

TEST(MessageTest, UuidFormat) {
    auto msg = Message::make_text("peer", "nick", "0000", "test");
    // UUID v4: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx (36 chars)
    EXPECT_EQ(msg.id.size(), 36u);
    EXPECT_EQ(msg.id[8], '-');
    EXPECT_EQ(msg.id[13], '-');
    EXPECT_EQ(msg.id[14], '4');
    EXPECT_EQ(msg.id[18], '-');
    EXPECT_EQ(msg.id[23], '-');
}

TEST(MessageTest, JsonFieldsPresent) {
    auto msg = Message::make_text("peer-1", "alice", "1530", "hi");
    auto j = msg.to_json();

    EXPECT_TRUE(j.contains("type"));
    EXPECT_TRUE(j.contains("id"));
    EXPECT_TRUE(j.contains("sender"));
    EXPECT_TRUE(j.contains("nickname"));
    EXPECT_TRUE(j.contains("tag"));
    EXPECT_TRUE(j.contains("body"));
    EXPECT_TRUE(j.contains("timestamp"));
    EXPECT_EQ(j["type"], "text");
    EXPECT_EQ(j["tag"], "1530");
}
