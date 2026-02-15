#include "peerchat/message.hpp"

#include <chrono>
#include <random>
#include <sstream>
#include <stdexcept>

namespace peerchat {

namespace {

std::string generate_uuid() {
    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<uint32_t> dist(0, 15);
    std::uniform_int_distribution<uint32_t> dist2(8, 11);

    const char* hex = "0123456789abcdef";
    std::string uuid(36, '-');
    // xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
    for (int i = 0; i < 36; ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) continue;
        if (i == 14) {
            uuid[i] = '4';
        } else if (i == 19) {
            uuid[i] = hex[dist2(rng)];
        } else {
            uuid[i] = hex[dist(rng)];
        }
    }
    return uuid;
}

int64_t now_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

} // namespace

std::string message_type_to_string(MessageType type) {
    switch (type) {
        case MessageType::Handshake: return "handshake";
        case MessageType::Text: return "text";
        case MessageType::Ack: return "ack";
        case MessageType::Ping: return "ping";
        case MessageType::Pong: return "pong";
    }
    return "unknown";
}

MessageType message_type_from_string(const std::string& s) {
    if (s == "handshake") return MessageType::Handshake;
    if (s == "text") return MessageType::Text;
    if (s == "ack") return MessageType::Ack;
    if (s == "ping") return MessageType::Ping;
    if (s == "pong") return MessageType::Pong;
    throw std::invalid_argument("Unknown message type: " + s);
}

nlohmann::json Message::to_json() const {
    nlohmann::json j;
    j["type"] = message_type_to_string(type);
    j["id"] = id;
    j["sender"] = sender;
    j["nickname"] = nickname;
    j["body"] = body;
    j["timestamp"] = timestamp;
    return j;
}

Message Message::from_json(const nlohmann::json& j) {
    Message m;
    m.type = message_type_from_string(j.at("type").get<std::string>());
    m.id = j.at("id").get<std::string>();
    m.sender = j.at("sender").get<std::string>();
    m.nickname = j.at("nickname").get<std::string>();
    m.body = j.at("body").get<std::string>();
    m.timestamp = j.at("timestamp").get<int64_t>();
    return m;
}

std::string Message::serialize() const { return to_json().dump(); }

Message Message::deserialize(const std::string& data) {
    return from_json(nlohmann::json::parse(data));
}

Message Message::make_handshake(const std::string& peer_id,
                                const std::string& nick) {
    Message m;
    m.type = MessageType::Handshake;
    m.id = generate_uuid();
    m.sender = peer_id;
    m.nickname = nick;
    m.timestamp = now_ms();
    return m;
}

Message Message::make_text(const std::string& peer_id, const std::string& nick,
                           const std::string& body) {
    Message m;
    m.type = MessageType::Text;
    m.id = generate_uuid();
    m.sender = peer_id;
    m.nickname = nick;
    m.body = body;
    m.timestamp = now_ms();
    return m;
}

Message Message::make_ack(const std::string& peer_id,
                          const std::string& msg_id) {
    Message m;
    m.type = MessageType::Ack;
    m.id = msg_id;
    m.sender = peer_id;
    m.timestamp = now_ms();
    return m;
}

Message Message::make_ping(const std::string& peer_id) {
    Message m;
    m.type = MessageType::Ping;
    m.id = generate_uuid();
    m.sender = peer_id;
    m.timestamp = now_ms();
    return m;
}

Message Message::make_pong(const std::string& peer_id) {
    Message m;
    m.type = MessageType::Pong;
    m.id = generate_uuid();
    m.sender = peer_id;
    m.timestamp = now_ms();
    return m;
}

} // namespace peerchat
