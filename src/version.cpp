#include "peerchat/version.hpp"
#include "peerchat/git_version.hpp"

#include <cstdio>
#include <sstream>
#include <stdexcept>

namespace peerchat {

std::string Version::string() {
    std::ostringstream ss;
    ss << major << "." << minor << "." << patch;
    return ss.str();
}

std::string Version::full_string() {
    return string() + " (" + commit_hash() + ")";
}

const char* Version::commit_hash() {
    return PEERCHAT_GIT_HASH;
}

Version Version::parse(const std::string& s) {
    std::string input = s;
    if (!input.empty() && input[0] == 'v') {
        input = input.substr(1);
    }

    int a = 0, b = 0, c = 0;
    if (std::sscanf(input.c_str(), "%d.%d.%d", &a, &b, &c) < 2) {
        throw std::invalid_argument("Invalid version string: " + s);
    }
    return Version{a, b, c};
}

} // namespace peerchat
