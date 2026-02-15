#include "peerchat/version.hpp"

#include <sstream>

namespace peerchat {

std::string Version::string() {
    std::ostringstream ss;
    ss << major << "." << minor << "." << patch;
    return ss.str();
}

} // namespace peerchat
