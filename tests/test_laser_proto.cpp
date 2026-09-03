#include <gtest/gtest.h>
#include <cstring>

extern "C" {
#include "laser_proto.h"
}

TEST(LaserProto, RoundTrip) {
    const uint8_t msg[] = "Hi!";
    uint8_t frame[MAX_FRAME];

    size_t n = build_frame(msg, 3, frame, sizeof(frame));
    ASSERT_EQ(n, 7u);              // 2 preamb + LEN + 3 data + sum

    parser_t p;
    parser_init(&p);

    for (size_t i = 0; i < n - 1; i++) {
        EXPECT_EQ(feed_byte(&p, frame[i]), IDLE);
    }

    // CHECKSUM
    EXPECT_EQ(feed_byte(&p, frame[n - 1]), PACKET);

    EXPECT_EQ(p.length, 3);
    EXPECT_EQ(std::memcmp(p.buffer, msg, 3), 0);
}