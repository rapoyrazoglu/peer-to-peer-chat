#include "peerchat/version.hpp"

#include <spdlog/spdlog.h>

#include <iostream>

int main() {
    spdlog::info("PeerChat v{}", peerchat::Version::string());
    spdlog::info("Decentralized peer-to-peer chat");

    std::cout << "PeerChat v" << peerchat::Version::string() << "\n";
    std::cout << "Nothing here yet. Development in progress...\n";

    return 0;
}
