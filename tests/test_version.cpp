#include "peerchat/version.hpp"

#include <cstring>

#include <gtest/gtest.h>

TEST(VersionTest, MajorMinorPatch) {
    EXPECT_EQ(peerchat::Version::major, 0);
    EXPECT_EQ(peerchat::Version::minor, 2);
    EXPECT_EQ(peerchat::Version::patch, 0);
}

TEST(VersionTest, VersionString) {
    EXPECT_EQ(peerchat::Version::string(), "0.2.0");
}

TEST(VersionTest, CommitHashNotEmpty) {
    EXPECT_GT(std::strlen(peerchat::Version::commit_hash()), 0u);
}

TEST(VersionTest, FullStringContainsHash) {
    auto full = peerchat::Version::full_string();
    EXPECT_NE(full.find(peerchat::Version::commit_hash()), std::string::npos);
}

TEST(VersionTest, ParseAndCompare) {
    auto v1 = peerchat::Version::parse("v0.1.0");
    auto v2 = peerchat::Version::parse("0.2.0");
    EXPECT_TRUE(v1 < v2);
    EXPECT_EQ(v2, peerchat::Version::current());
}
