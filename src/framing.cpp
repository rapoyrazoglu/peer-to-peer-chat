#include "peerchat/framing.hpp"

#include <cstring>
#include <stdexcept>

namespace peerchat {

std::vector<uint8_t> FrameEncoder::encode(const std::string& payload) {
    uint32_t len = static_cast<uint32_t>(payload.size());
    if (len > kMaxFrameSize) {
        throw std::length_error("Payload exceeds max frame size");
    }

    std::vector<uint8_t> frame(4 + len);
    // Big-endian length prefix
    frame[0] = static_cast<uint8_t>((len >> 24) & 0xFF);
    frame[1] = static_cast<uint8_t>((len >> 16) & 0xFF);
    frame[2] = static_cast<uint8_t>((len >> 8) & 0xFF);
    frame[3] = static_cast<uint8_t>(len & 0xFF);
    std::memcpy(frame.data() + 4, payload.data(), len);
    return frame;
}

void FrameDecoder::feed(const uint8_t* data, std::size_t len) {
    buffer_.insert(buffer_.end(), data, data + len);
}

std::optional<std::string> FrameDecoder::next() {
    if (buffer_.size() < 4) return std::nullopt;

    uint32_t len = (static_cast<uint32_t>(buffer_[0]) << 24) |
                   (static_cast<uint32_t>(buffer_[1]) << 16) |
                   (static_cast<uint32_t>(buffer_[2]) << 8) |
                   static_cast<uint32_t>(buffer_[3]);

    if (len > kMaxFrameSize) {
        throw std::length_error("Received frame exceeds max size");
    }

    if (buffer_.size() < 4 + len) return std::nullopt;

    std::string payload(buffer_.begin() + 4, buffer_.begin() + 4 + len);
    buffer_.erase(buffer_.begin(), buffer_.begin() + 4 + len);
    return payload;
}

} // namespace peerchat
