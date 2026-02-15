#include "peerchat/identity.hpp"

#include <cstdlib>
#include <fstream>
#include <random>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace peerchat {

Identity::Identity(const std::string& nickname) : nickname_(nickname) {
    if (!load()) {
        peer_id_ = generate_uuid();
        spdlog::info("Generated new peer ID: {}", peer_id_);
        save();
    } else {
        spdlog::info("Loaded peer ID: {}", peer_id_);
        if (!nickname.empty()) {
            nickname_ = nickname;
        }
    }
}

std::string Identity::generate_uuid() {
    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<uint32_t> dist(0, 15);
    std::uniform_int_distribution<uint32_t> dist2(8, 11);

    const char* hex = "0123456789abcdef";
    std::string uuid(36, '-');
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

std::filesystem::path Identity::config_dir() {
#ifdef _WIN32
    const char* home = std::getenv("USERPROFILE");
#else
    const char* home = std::getenv("HOME");
#endif
    if (!home) {
        return std::filesystem::current_path() / ".peerchat";
    }
    return std::filesystem::path(home) / ".peerchat";
}

std::filesystem::path Identity::identity_path() const {
    return config_dir() / "identity.json";
}

void Identity::save() const {
    auto dir = config_dir();
    std::filesystem::create_directories(dir);

    nlohmann::json j;
    j["peer_id"] = peer_id_;
    j["nickname"] = nickname_;

    std::ofstream f(identity_path());
    if (f.is_open()) {
        f << j.dump(2);
        spdlog::debug("Identity saved to {}", identity_path().string());
    } else {
        spdlog::warn("Failed to save identity to {}",
                     identity_path().string());
    }
}

bool Identity::load() {
    auto path = identity_path();
    if (!std::filesystem::exists(path)) return false;

    std::ifstream f(path);
    if (!f.is_open()) return false;

    try {
        auto j = nlohmann::json::parse(f);
        peer_id_ = j.at("peer_id").get<std::string>();
        nickname_ = j.at("nickname").get<std::string>();
        return true;
    } catch (const std::exception& e) {
        spdlog::warn("Failed to parse identity file: {}", e.what());
        return false;
    }
}

} // namespace peerchat
