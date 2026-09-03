#include <gtest/gtest.h>
#include <cstring>

extern "C" {
#include "laser_proto.h"
}

// ─────────────────────────────────────────────────────────────
//  cycle for() for feeding parser
// ─────────────────────────────────────────────────────────────

static result_t feed_all(parser_t &p, const uint8_t *data, size_t n) {
    result_t r = IDLE;
    for (size_t i = 0; i < n; i++) {
        r = feed_byte(&p, data[i]);
    }
    return r;
}

// ─────────────────────────────────────────────────────────────
//  basic
// ─────────────────────────────────────────────────────────────

TEST(LaserProto, RoundTrip) {
    const uint8_t msg[] = "Hi!";
    uint8_t frame[MAX_FRAME];

    size_t n = build_frame(msg, 3, frame, sizeof(frame));
    ASSERT_EQ(n, 7u);              // AA 55 LEN + 3 данных + сумма

    parser_t p;
    parser_init(&p);

    EXPECT_EQ(feed_all(p, frame, n), PACKET);
    EXPECT_EQ(p.length, 3);
    EXPECT_EQ(std::memcmp(p.buffer, msg, 3), 0);
}

// ─────────────────────────────────────────────────────────────
//   build_frame() bad arguments
// ─────────────────────────────────────────────────────────────

TEST(LaserProto, BuildFrameRejectsBadArgs) {
    uint8_t payload[100];
    uint8_t out[MAX_FRAME];

    std::memset(payload, 0x42, sizeof(payload));

    EXPECT_EQ(build_frame(nullptr, 3, out, sizeof(out)), 0u);
    EXPECT_EQ(build_frame(payload, 0, out, sizeof(out)), 0u);
    EXPECT_EQ(build_frame(payload, 3, nullptr, sizeof(out)), 0u);
    EXPECT_EQ(build_frame(payload, 3, out, sizeof(nullptr)), 0u);
    EXPECT_EQ(build_frame(payload, 3, out, 5), 0u );
}

// ─────────────────────────────────────────────────────────────
//  BAD_CRC
// ─────────────────────────────────────────────────────────────

TEST(LaserProto, BadChecksumIsRejected) {
    const uint8_t msg[] = "Hi!";
    uint8_t frame[MAX_FRAME];

    size_t n = build_frame(msg, 3, frame, sizeof(frame));
    ASSERT_EQ(n, 7u);

    frame[n - 1] ^= 0xFF;

    parser_t p;
    parser_init(&p);

    EXPECT_EQ(feed_all(p, frame, n), BAD_CRC);
}

TEST(LaserProto, ZeroLengthIsRejected) {
    const uint8_t bytes[] = { 0xAA, 0x55, 0x00 };

    parser_t p;
    parser_init(&p);

    EXPECT_EQ(feed_all(p, bytes, sizeof(bytes)), BAD_LEN);
}

TEST(LaserProto, TooBigLengthIsRejected) {
    const uint8_t bytes[] = { 0xAA, 0x55, 65 };

    parser_t p;
    parser_init(&p);

    EXPECT_EQ(feed_all(p, bytes, sizeof(bytes)), BAD_LEN);
}
// ─────────────────────────────────────────────────────────────
//  sync
// ─────────────────────────────────────────────────────────────



// ─────────────────────────────────────────────────────────────
//  restore after bad signal
// ─────────────────────────────────────────────────────────────

