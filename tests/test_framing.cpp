#include "peerchat/framing.hpp"

#include <gtest/gtest.h>

using namespace peerchat;

TEST(FramingTest, EncodeDecodeSimple) {
    std::string payload = R"({"type":"text","body":"hello"})";
    auto frame = FrameEncoder::encode(payload);

    FrameDecoder decoder;
    decoder.feed(frame.data(), frame.size());

    auto result = decoder.next();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, payload);
    EXPECT_EQ(decoder.buffered(), 0u);
}

TEST(FramingTest, PartialFrame) {
    std::string payload = "hello world";
    auto frame = FrameEncoder::encode(payload);

    FrameDecoder decoder;
    // Feed only the header first
    decoder.feed(frame.data(), 4);
    EXPECT_FALSE(decoder.next().has_value());

    // Feed partial payload
    decoder.feed(frame.data() + 4, 5);
    EXPECT_FALSE(decoder.next().has_value());

    // Feed remaining payload
    decoder.feed(frame.data() + 9, frame.size() - 9);
    auto result = decoder.next();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, payload);
}

TEST(FramingTest, MultipleFrames) {
    std::string p1 = "frame one";
    std::string p2 = "frame two";
    auto f1 = FrameEncoder::encode(p1);
    auto f2 = FrameEncoder::encode(p2);

    // Concatenate both frames
    std::vector<uint8_t> combined;
    combined.insert(combined.end(), f1.begin(), f1.end());
    combined.insert(combined.end(), f2.begin(), f2.end());

    FrameDecoder decoder;
    decoder.feed(combined.data(), combined.size());

    auto r1 = decoder.next();
    ASSERT_TRUE(r1.has_value());
    EXPECT_EQ(*r1, p1);

    auto r2 = decoder.next();
    ASSERT_TRUE(r2.has_value());
    EXPECT_EQ(*r2, p2);

    EXPECT_FALSE(decoder.next().has_value());
}

TEST(FramingTest, EmptyPayload) {
    auto frame = FrameEncoder::encode("");
    FrameDecoder decoder;
    decoder.feed(frame.data(), frame.size());

    auto result = decoder.next();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "");
}

TEST(FramingTest, BigEndianEncoding) {
    std::string payload(256, 'x');
    auto frame = FrameEncoder::encode(payload);

    // Length = 256 = 0x00000100
    EXPECT_EQ(frame[0], 0x00);
    EXPECT_EQ(frame[1], 0x00);
    EXPECT_EQ(frame[2], 0x01);
    EXPECT_EQ(frame[3], 0x00);
}

TEST(FramingTest, OversizedPayloadThrows) {
    std::string huge(kMaxFrameSize + 1, 'x');
    EXPECT_THROW(FrameEncoder::encode(huge), std::length_error);
}

TEST(FramingTest, ByteByByteFeed) {
    std::string payload = "abc";
    auto frame = FrameEncoder::encode(payload);

    FrameDecoder decoder;
    for (std::size_t i = 0; i < frame.size() - 1; ++i) {
        decoder.feed(&frame[i], 1);
        EXPECT_FALSE(decoder.next().has_value());
    }
    decoder.feed(&frame.back(), 1);
    auto result = decoder.next();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, payload);
}
