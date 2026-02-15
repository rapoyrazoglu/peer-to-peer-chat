#include "peerchat/version.hpp"

#include <gtest/gtest.h>

TEST(VersionTest, MajorMinorPatch) {
    EXPECT_EQ(peerchat::Version::major, 0);
    EXPECT_EQ(peerchat::Version::minor, 1);
    EXPECT_EQ(peerchat::Version::patch, 0);
}

TEST(VersionTest, VersionString) {
    EXPECT_EQ(peerchat::Version::string(), "0.1.0");
}
