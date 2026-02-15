#include "peerchat/identity.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

using namespace peerchat;

namespace {

void set_env(const char* name, const char* value) {
#ifdef _WIN32
    _putenv_s(name, value);
#else
    setenv(name, value, 1);
#endif
}

const char* home_var() {
#ifdef _WIN32
    return "USERPROFILE";
#else
    return "HOME";
#endif
}

} // namespace

class IdentityTest : public ::testing::Test {
  protected:
    void SetUp() override {
        test_dir_ = std::filesystem::temp_directory_path() / "peerchat_test";
        std::filesystem::create_directories(test_dir_);
        const char* h = std::getenv(home_var());
        original_home_ = h ? h : "";
        set_env(home_var(), test_dir_.string().c_str());
    }

    void TearDown() override {
        std::filesystem::remove_all(test_dir_);
        if (!original_home_.empty()) {
            set_env(home_var(), original_home_.c_str());
        }
    }

    std::filesystem::path test_dir_;
    std::string original_home_;
};

TEST_F(IdentityTest, GeneratesNewIdentity) {
    Identity id("alice");

    EXPECT_FALSE(id.peer_id().empty());
    EXPECT_EQ(id.nickname(), "alice");
    // UUID v4 format
    EXPECT_EQ(id.peer_id().size(), 36u);
    EXPECT_EQ(id.peer_id()[14], '4');

    // Tag is 4 digits
    EXPECT_EQ(id.tag().size(), 4u);
    for (char c : id.tag()) {
        EXPECT_TRUE(c >= '0' && c <= '9');
    }
}

TEST_F(IdentityTest, SaveAndLoad) {
    std::string saved_peer_id;
    std::string saved_tag;
    {
        Identity id("bob");
        saved_peer_id = id.peer_id();
        saved_tag = id.tag();
        EXPECT_FALSE(saved_peer_id.empty());
        EXPECT_FALSE(saved_tag.empty());
    }

    Identity id2("charlie");
    EXPECT_EQ(id2.peer_id(), saved_peer_id);
    EXPECT_EQ(id2.tag(), saved_tag);
    EXPECT_EQ(id2.nickname(), "charlie");
}

TEST_F(IdentityTest, IdentityFileExists) {
    Identity id("dave");
    auto path = Identity::config_dir() / "identity.json";
    EXPECT_TRUE(std::filesystem::exists(path));
}

TEST_F(IdentityTest, UniqueIds) {
    std::string id1;
    {
        auto dir1 = test_dir_ / "home1";
        std::filesystem::create_directories(dir1);
        set_env(home_var(), dir1.string().c_str());
        Identity identity1("a");
        id1 = identity1.peer_id();
    }

    std::string id2;
    {
        auto dir2 = test_dir_ / "home2";
        std::filesystem::create_directories(dir2);
        set_env(home_var(), dir2.string().c_str());
        Identity identity2("b");
        id2 = identity2.peer_id();
    }

    EXPECT_NE(id1, id2);
}

TEST_F(IdentityTest, DisplayNameFormat) {
    Identity id("alice");
    auto dn = id.display_name();
    // Should be "alice#XXXX"
    EXPECT_EQ(dn.substr(0, 6), "alice#");
    EXPECT_EQ(dn.size(), 10u);
}
