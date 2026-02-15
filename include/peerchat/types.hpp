#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace peerchat {

class Connection;
using ConnectionPtr = std::shared_ptr<Connection>;

using MessageCallback = std::function<void(const std::string& json)>;
using ErrorCallback = std::function<void(const std::string& reason)>;
using ConnectCallback = std::function<void(ConnectionPtr conn)>;
using DisconnectCallback = std::function<void(const std::string& reason)>;

static constexpr std::size_t kMaxFrameSize = 64 * 1024; // 64 KiB
static constexpr uint16_t kDefaultPort = 9000;

} // namespace peerchat
