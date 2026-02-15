#pragma once

#include <string>

namespace peerchat {

struct UpdateInfo {
    bool available{false};
    std::string current_version;
    std::string latest_version;
    std::string asset_url;
    std::string asset_name;
};

class Updater {
  public:
    // Check for updates. If beta is true, include pre-releases.
    static UpdateInfo check(bool beta = false);

    // Download and install update. Returns true on success.
    static bool perform(bool beta = false);

    // Remove PeerChat completely: config, temp files, and binary.
    static bool uninstall();

  private:
    static std::string platform_asset_name();
    static bool install_asset(const std::string& path,
                              const std::string& asset_name);
};

} // namespace peerchat
