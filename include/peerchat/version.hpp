#pragma once

#include <string>
#include <tuple>

namespace peerchat {

struct Version {
    static constexpr int major = 0;
    static constexpr int minor = 2;
    static constexpr int patch = 0;

    int maj{0};
    int min{0};
    int pat{0};

    // "0.2.0"
    static std::string string();

    // "0.2.0 (a3deab2)"
    static std::string full_string();

    // Build commit hash
    static const char* commit_hash();

    // Parse version from string like "0.2.0" or "v0.2.0"
    static Version parse(const std::string& s);

    // Current version as a Version instance
    static Version current() { return Version{major, minor, patch}; }

    bool operator<(const Version& o) const {
        return std::tie(maj, min, pat) < std::tie(o.maj, o.min, o.pat);
    }
    bool operator==(const Version& o) const {
        return std::tie(maj, min, pat) == std::tie(o.maj, o.min, o.pat);
    }
    bool operator!=(const Version& o) const { return !(*this == o); }
};

} // namespace peerchat
