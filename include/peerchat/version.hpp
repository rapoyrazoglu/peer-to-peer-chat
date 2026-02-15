#pragma once

#include <string>

namespace peerchat {

struct Version {
    static constexpr int major = 0;
    static constexpr int minor = 1;
    static constexpr int patch = 0;

    static std::string string();
};

} // namespace peerchat
