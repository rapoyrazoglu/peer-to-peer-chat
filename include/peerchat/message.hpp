#pragma once

#include <cstdint>
#include <string>

#include <nlohmann/json.hpp>

namespace peerchat {

enum class MessageType : uint8_t {
    Handshake,
    Text,
    Ack,
    Ping,
    Pong,
};

std::string message_type_to_string(MessageType type);
MessageType message_type_from_string(const std::string& s);

struct Message {
    MessageType type;
    std::string id;       // UUID for text messages, echoed in ACKs
    std::string sender;   // peer ID
    std::string nickname; // display name
    std::string tag;      // 4-digit identity tag
    std::string body;     // text content (for Text), empty for control msgs
    int64_t timestamp{0}; // Unix epoch milliseconds

    nlohmann::json to_json() const;
    static Message from_json(const nlohmann::json& j);

    std::string serialize() const;
    static Message deserialize(const std::string& data);

    static Message make_handshake(const std::string& peer_id,
                                  const std::string& nick,
                                  const std::string& tag);
    static Message make_text(const std::string& peer_id,
                             const std::string& nick,
                             const std::string& tag,
                             const std::string& body);
    static Message make_ack(const std::string& peer_id,
                            const std::string& msg_id);
    static Message make_ping(const std::string& peer_id);
    static Message make_pong(const std::string& peer_id);
};

} // namespace peerchat
