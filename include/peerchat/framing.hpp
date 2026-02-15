#pragma once

#include "peerchat/types.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace peerchat {

// Encodes a JSON string into a length-prefixed frame:
// [4-byte big-endian length][payload]
struct FrameEncoder {
    static std::vector<uint8_t> encode(const std::string& payload);
};

// Stateful decoder that accumulates bytes and yields complete frames.
// Handles partial reads across multiple feed() calls.
class FrameDecoder {
  public:
    // Feed raw bytes into the decoder
    void feed(const uint8_t* data, std::size_t len);

    // Try to extract the next complete frame.
    // Returns nullopt if no complete frame is available yet.
    std::optional<std::string> next();

    // Number of buffered bytes not yet consumed
    std::size_t buffered() const { return buffer_.size(); }

  private:
    std::vector<uint8_t> buffer_;
};

} // namespace peerchat
