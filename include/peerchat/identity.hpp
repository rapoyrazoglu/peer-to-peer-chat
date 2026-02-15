#pragma once

#include <filesystem>
#include <string>

namespace peerchat {

class Identity {
  public:
    // Load or create identity. If config dir has an existing identity, load it.
    // Otherwise generate a new UUID and save.
    explicit Identity(const std::string& nickname);

    const std::string& peer_id() const { return peer_id_; }
    const std::string& nickname() const { return nickname_; }
    const std::string& tag() const { return tag_; }
    std::string display_name() const { return nickname_ + "#" + tag_; }
    void set_nickname(const std::string& nick) { nickname_ = nick; }

    // Save identity to disk (~/.peerchat/identity.json)
    void save() const;

    // Load identity from disk. Returns false if file doesn't exist.
    bool load();

    // Get the config directory path
    static std::filesystem::path config_dir();

  private:
    std::string peer_id_;
    std::string nickname_;
    std::string tag_;

    static std::string generate_uuid();
    static std::string generate_tag();
    std::filesystem::path identity_path() const;
};

} // namespace peerchat
