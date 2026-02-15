#include "peerchat/updater.hpp"
#include "peerchat/identity.hpp"
#include "peerchat/version.hpp"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <httplib.h>
#include <nlohmann/json.hpp>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace peerchat {

namespace {

const std::string kGitHubAPI = "https://api.github.com";
const std::string kRepoPath = "/repos/rapoyrazoglu/peer-to-peer-chat";

// Parse host and path from a full URL like "https://host/path..."
std::pair<std::string, std::string> split_url(const std::string& url) {
    // Find "://"
    auto scheme_end = url.find("://");
    if (scheme_end == std::string::npos) {
        return {"", url};
    }
    auto host_start = scheme_end + 3;
    auto path_start = url.find('/', host_start);
    if (path_start == std::string::npos) {
        return {url.substr(0, host_start) + url.substr(host_start), "/"};
    }
    std::string origin = url.substr(0, path_start);
    std::string path = url.substr(path_start);
    return {origin, path};
}

std::string download_to_temp(const std::string& url) {
    auto tmp = std::filesystem::temp_directory_path() / "peerchat_update";
    std::filesystem::create_directories(tmp);

    auto [origin, path] = split_url(url);
    if (origin.empty()) {
        throw std::runtime_error("Invalid download URL: " + url);
    }

    httplib::Client dl(origin);
    dl.set_follow_location(true);
    dl.set_connection_timeout(30);
    dl.set_read_timeout(120);

    auto res = dl.Get(path);
    if (!res) {
        throw std::runtime_error("Download failed: connection error");
    }
    if (res->status != 200) {
        throw std::runtime_error("Download failed: HTTP " +
                                 std::to_string(res->status));
    }

    // Determine filename from URL
    auto slash = url.rfind('/');
    std::string filename =
        (slash != std::string::npos) ? url.substr(slash + 1) : "update";
    auto filepath = tmp / filename;

    std::ofstream f(filepath, std::ios::binary);
    if (!f.is_open()) {
        throw std::runtime_error("Failed to create temp file: " +
                                 filepath.string());
    }
    f.write(res->body.data(),
            static_cast<std::streamsize>(res->body.size()));
    f.close();
    if (!f.good()) {
        std::filesystem::remove(filepath);
        throw std::runtime_error("Failed to write downloaded data");
    }

    return filepath.string();
}

} // namespace

std::string Updater::platform_asset_name() {
#if defined(_WIN32)
    return "peerchat-setup.exe";
#elif defined(__APPLE__)
  #if defined(__aarch64__) || defined(__arm64__)
    return "peerchat-macos-arm64.pkg";
  #else
    return "peerchat-macos-x86_64.pkg";
  #endif
#elif defined(__linux__)
    return "peerchat-linux-x86_64.tar.gz";
#else
    return "";
#endif
}

UpdateInfo Updater::check(bool beta) {
    UpdateInfo info;
    info.current_version = Version::string();

    httplib::Client client(kGitHubAPI);
    client.set_follow_location(true);
    client.set_connection_timeout(10);
    client.set_read_timeout(10);

    httplib::Headers headers = {
        {"Accept", "application/vnd.github+json"},
        {"User-Agent", "PeerChat/" + Version::string()},
    };

    std::string endpoint =
        beta ? kRepoPath + "/releases" : kRepoPath + "/releases/latest";

    auto res = client.Get(endpoint, headers);
    if (!res) {
        std::cerr << "Failed to check for updates: connection error\n";
        return info;
    }
    if (res->status != 200) {
        std::cerr << "Failed to check for updates (HTTP "
                  << res->status << ")\n";
        return info;
    }

    try {
        nlohmann::json release;
        if (beta) {
            auto releases = nlohmann::json::parse(res->body);
            if (!releases.is_array() || releases.empty()) return info;
            release = releases[0];
        } else {
            release = nlohmann::json::parse(res->body);
        }

        auto tag = release.at("tag_name").get<std::string>();
        info.latest_version = tag;

        // Get the commit hash from the release
        if (release.contains("target_commitish")) {
            info.latest_commit =
                release.at("target_commitish").get<std::string>();
        }

        auto latest = Version::parse(tag);
        auto current = Version::current();
        auto local_hash = std::string(Version::commit_hash());

        bool newer_version = current < latest;
        bool same_version_different_build =
            (current == latest) && !info.latest_commit.empty() &&
            !local_hash.empty() && local_hash != "unknown" &&
            info.latest_commit.find(local_hash) == std::string::npos &&
            local_hash.find(info.latest_commit) == std::string::npos;

        if (newer_version || same_version_different_build) {
            info.available = true;
            info.same_version_rebuild = same_version_different_build;

            auto wanted = platform_asset_name();
            if (wanted.empty()) {
                std::cerr << "Unsupported platform for auto-update\n";
                return info;
            }

            if (release.contains("assets") && release["assets"].is_array()) {
                for (auto& asset : release["assets"]) {
                    auto name = asset.at("name").get<std::string>();
                    if (name == wanted) {
                        info.asset_url =
                            asset.at("browser_download_url")
                                .get<std::string>();
                        info.asset_name = name;
                        break;
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse release info: " << e.what() << "\n";
    }

    return info;
}

bool Updater::perform(bool beta) {
    std::cout << "Checking for updates... (current: v"
              << Version::full_string() << ")\n";
    auto info = check(beta);

    if (!info.available) {
        std::cout << "Already up to date (v" << info.current_version << ")\n";
        return true;
    }

    if (info.asset_url.empty()) {
        std::cerr << "Update available (" << info.latest_version
                  << ") but no compatible asset found for this platform\n";
        return false;
    }

    if (info.same_version_rebuild) {
        std::cout << "Rebuild available: same version but different build\n";
        std::cout << "  Local:  " << Version::commit_hash() << "\n";
        std::cout << "  Remote: " << info.latest_commit.substr(0, 7) << "\n";
    } else {
        std::cout << "Updating: v" << info.current_version << " -> "
                  << info.latest_version << "\n";
    }
    std::cout << "Downloading " << info.asset_name << "...\n";

    try {
        auto path = download_to_temp(info.asset_url);
        std::cout << "Installing...\n";

        if (!install_asset(path, info.asset_name)) {
            std::cerr << "Installation failed\n";
            return false;
        }

        std::cout << "Update successful! Restart PeerChat to use "
                  << info.latest_version << "\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Update failed: " << e.what() << "\n";
        return false;
    }
}

bool Updater::install_asset(const std::string& path,
                            const std::string& /*asset_name*/) {
#if defined(__linux__)
    // Extract tar.gz and replace binary
    auto exe = std::filesystem::read_symlink("/proc/self/exe");
    auto tmp_dir = std::filesystem::path(path).parent_path();
    std::string cmd = "tar xzf '" + path + "' -C '" + tmp_dir.string() + "'";
    if (std::system(cmd.c_str()) != 0) return false;

    auto extracted = tmp_dir / "peerchat";
    if (!std::filesystem::exists(extracted)) return false;

    // Use copy + remove instead of rename (works across filesystems)
    std::error_code ec;
    std::filesystem::copy_file(extracted, exe,
                               std::filesystem::copy_options::overwrite_existing,
                               ec);
    if (ec) {
        std::cerr << "Failed to replace binary: " << ec.message() << "\n";
        return false;
    }

    // Set executable permission
    std::filesystem::permissions(exe, std::filesystem::perms::owner_exec |
                                         std::filesystem::perms::group_exec |
                                         std::filesystem::perms::others_exec,
                                 std::filesystem::perm_options::add, ec);

    // Clean up temp files
    std::filesystem::remove(extracted, ec);
    std::filesystem::remove(path, ec);
    return true;
#elif defined(__APPLE__)
    std::string cmd = "sudo installer -pkg '" + path + "' -target /";
    return std::system(cmd.c_str()) == 0;
#elif defined(_WIN32)
    std::string cmd = "\"" + path + "\" /S";
    return std::system(cmd.c_str()) == 0;
#else
    (void)path;
    std::cerr << "Auto-update not supported on this platform\n";
    return false;
#endif
}

bool Updater::uninstall() {
    std::cout << "Uninstalling PeerChat...\n";

    bool success = true;
    std::error_code ec;

    // 1. Remove config directory (~/.peerchat)
    auto config = Identity::config_dir();
    if (std::filesystem::exists(config)) {
        std::filesystem::remove_all(config, ec);
        if (ec) {
            std::cerr << "Failed to remove config dir " << config << ": "
                      << ec.message() << "\n";
            success = false;
        } else {
            std::cout << "Removed config: " << config << "\n";
        }
    }

    // 2. Remove temp update files
    auto tmp = std::filesystem::temp_directory_path() / "peerchat_update";
    if (std::filesystem::exists(tmp)) {
        std::filesystem::remove_all(tmp, ec);
        if (ec) {
            std::cerr << "Failed to remove temp dir: " << ec.message() << "\n";
        } else {
            std::cout << "Removed temp files: " << tmp << "\n";
        }
    }

    // 3. Remove the binary itself
    std::filesystem::path exe;
#if defined(__linux__)
    exe = std::filesystem::read_symlink("/proc/self/exe", ec);
    if (ec) exe.clear();
#elif defined(__APPLE__)
    {
        char buf[4096];
        uint32_t size = sizeof(buf);
        if (_NSGetExecutablePath(buf, &size) == 0) {
            exe = std::filesystem::canonical(buf, ec);
            if (ec) exe.clear();
        }
    }
#elif defined(_WIN32)
    {
        char buf[MAX_PATH];
        if (GetModuleFileNameA(nullptr, buf, MAX_PATH) > 0) {
            exe = buf;
        }
    }
#endif

    if (!exe.empty() && std::filesystem::exists(exe)) {
        std::filesystem::remove(exe, ec);
        if (ec) {
            std::cerr << "Failed to remove binary " << exe << ": "
                      << ec.message() << "\n";
            std::cout << "Remove it manually: rm " << exe << "\n";
            success = false;
        } else {
            std::cout << "Removed binary: " << exe << "\n";
        }
    }

    if (success) {
        std::cout << "PeerChat has been completely uninstalled.\n";
    } else {
        std::cout << "Uninstall completed with some errors (see above).\n";
    }
    return success;
}

} // namespace peerchat
