#include <gtest/gtest.h>
#include <cstring>

extern "C" {
#include "laser_proto.h"
}

//  CYCLE for() FOR FEEDING PARSER
static result_t feed_all(parser_t &p, const uint8_t *data, size_t n) {
    result_t r = IDLE;
    for (size_t i = 0; i < n; i++) {
        r = feed_byte(&p, data[i], 0);
    }
    return r;
}

//  CYCLE for() FOR DELAYED FEEDING PARSER
static result_t feed_all_at(parser_t &p, const uint8_t *data, size_t n,
                            uint32_t t0, uint32_t step_ms)
{
    result_t r = IDLE;
    for (size_t i = 0; i < n; i++) {
        r = feed_byte(&p, data[i], t0 + (uint32_t)i * step_ms);
    }
    return r;
}

// ─────────────────────────────────────────────────────────────
//  BASIC
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
    uint8_t out[200];

    std::memset(payload, 0x42, sizeof(payload));

    EXPECT_EQ(build_frame(nullptr, 3, out, sizeof(out)), 0u);
    EXPECT_EQ(build_frame(payload, 0, out, sizeof(out)), 0u);
    EXPECT_EQ(build_frame(payload, 3, nullptr, sizeof(out)), 0u);
    EXPECT_EQ(build_frame(payload, 65, out, sizeof(out)), 0u);
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

TEST(LaserProto, GarbageBeforeFrameIsIgnored) {
    const uint8_t garbage[] = {0x11, 0x22, 0xFF, 0x00};
    const uint8_t msg[] = {"Hi!"};
    uint8_t frame[MAX_FRAME];

    size_t n = build_frame(msg, 3, frame, sizeof(frame));
    ASSERT_EQ(n, 7u);

    parser_t p;
    parser_init(&p);

    EXPECT_EQ(feed_all(p, garbage, sizeof(garbage)), IDLE);
    EXPECT_EQ(feed_all(p, frame, n), PACKET);
    EXPECT_EQ(p.length, 3);
}

TEST(LaserProto, DoubleSyncByteIsAccepted) {
    const uint8_t bytes[8] = {0xAA, 0xAA, 0x55, 0x03, 'H', 'i', '!', 0x03};

    parser_t p;
    parser_init(&p);

    EXPECT_EQ(feed_all(p, bytes, sizeof(bytes)), PACKET);
    EXPECT_EQ(p.length, 3);
    EXPECT_EQ(std::memcmp(p.buffer, "Hi!", 3), 0);
}
TEST(LaserProto, PayloadMayContainPreamble) {
    const uint8_t msg[] = { 0x01, 0xAA, 0x55, 0x02 };
    uint8_t frame[MAX_FRAME];

    size_t n = build_frame(msg, sizeof(msg), frame, sizeof(frame));
    ASSERT_EQ(n, 8u);

    parser_t p;
    parser_init(&p);

    EXPECT_EQ(feed_all(p, frame, n), PACKET);
    EXPECT_EQ(p.length, sizeof(msg));
    EXPECT_EQ(std::memcmp(p.buffer, msg, sizeof(msg)), 0);
}

// ─────────────────────────────────────────────────────────────
//  restore after bad frame
// ─────────────────────────────────────────────────────────────

TEST(LaserProto, RecoverAfterBadFrame) {
    const uint8_t trunc[] = {0xAA, 0x55, 0x05, 0x01, 0x02};
    const uint8_t msg[] = {"Hi!"};
    uint8_t frame[MAX_FRAME];
    size_t n = build_frame(msg, 3, frame, sizeof(frame));
    ASSERT_EQ(n, 7u);

    parser_t p;
    parser_init(&p);

    feed_all(p, trunc, sizeof(trunc));
    //protocol will NOT accept next frame without last 3 bytes
    EXPECT_NE(feed_all(p, frame, n), PACKET);
    //but will when next packet arrive
    EXPECT_EQ(feed_all(p, frame, n), PACKET);
    EXPECT_EQ(p.length, 3);
    EXPECT_EQ(std::memcmp(p.buffer, msg, 3), 0);
}

// ─────────────────────────────────────────────────────────────
//  TIMEOUT (VALID / BAD / now_ms OVERFLOW)
// ─────────────────────────────────────────────────────────────

TEST(LaserProto, NormalInterByteGapKeepsFrame) {
    uint8_t frame[MAX_FRAME];
    const uint8_t payload[] = {'h', 'i'};
    size_t n = build_frame(payload, sizeof(payload), frame, sizeof(frame));

    parser_t p;
    parser_init(&p);

    // WHOLE FRAME WITH 8 MS BETWEEN BYTES w/ ~8.3 MS AT 1200 BAUD MUST BE ACCEPTED
    EXPECT_EQ(feed_all_at(p, frame, n, 1000, 8), PACKET);
    EXPECT_EQ(p.length, sizeof(payload));
}

TEST(LaserProto, StalePartialFrameIsDroppedAfterTimeout) {
    uint8_t frame[MAX_FRAME];
    const uint8_t payload[] = {'h', 'i'};
    size_t n = build_frame(payload, sizeof(payload), frame, sizeof(frame));

    parser_t p;
    parser_init(&p);

    // FIRST 3 BYTES DELIVERED NORMALLY
    EXPECT_EQ(feed_all_at(p, frame, 3, 1000, 8), IDLE);

    // LAST BYTES AFTER A PAUSE > TIMEOUT MUST BE HANDLED
    // + 1 IS ABOVE INTERBYTE_TIMEOUT_MS
    uint32_t later = 1000 + 2 * 8 + INTERBYTE_TIMEOUT_MS + 1;
    EXPECT_EQ(feed_all_at(p, frame, n, later, 8), PACKET);
    EXPECT_EQ(p.length, sizeof(payload));
}

TEST(LaserProto, GapExactlyAtTimeoutKeepsFrame) {
    uint8_t frame[MAX_FRAME];
    const uint8_t payload[] = {'h', 'i'};
    size_t n = build_frame(payload, sizeof(payload), frame, sizeof(frame));

    parser_t p;
    parser_init(&p);

    // FIRST 3 BYTES DELIVERED NORMALLY
    EXPECT_EQ(feed_all_at(p, frame, 3, 1000, 8), IDLE);

    // LAST BYTES AFTER A PAUSE > TIMEOUT MUST BE ACCEPTED
    // REST OF THE SAME FRAME AFTER A PAUSE == TIMEOUT: NO RESET, FRAME ACCEPTED
    uint32_t t = 1016 + INTERBYTE_TIMEOUT_MS;
    EXPECT_EQ(feed_all_at(p, frame + 3, n - 3, t, 8), PACKET);
}

TEST(LaserProto, NormalGapAcrossTickWraparoundKeepsFrame) {
    uint8_t frame[MAX_FRAME];
    const uint8_t payload[] = {'h', 'i'};
    size_t n = build_frame(payload, sizeof(payload), frame, sizeof(frame));

    parser_t p;
    parser_init(&p);

    // BYTES AT 0xFFFFFFF0, 0xFFFFFFF8, 0x00000000, 0x00000008, ...
    // TICK WRAPS INSIDE THE FRAME, 8 MS GAPS MUST NOT RESET PARSER
    EXPECT_EQ(feed_all_at(p, frame, n, 0xFFFFFFF0u, 8), PACKET);
}

TEST(LaserProto, StaleFrameAcrossTickWraparoundIsDropped) {
    uint8_t frame[MAX_FRAME];
    const uint8_t payload[] = {'h', 'i'};
    size_t n = build_frame(payload, sizeof(payload), frame, sizeof(frame));

    parser_t p;
    parser_init(&p);

    // AA 55 LEN AT 0xFFFFFFE0, 0xFFFFFFE8, 0xFFFFFFF0, THEN SILENCE
    EXPECT_EQ(feed_all_at(p, frame, 3, 0xFFFFFFE0u, 8), IDLE);

    // NEW FRAME AFTER THE WRAP: 0xFFFFFFF0 + TIMEOUT + 1 = 0x00000005
    uint32_t later = 0xFFFFFFF0u + INTERBYTE_TIMEOUT_MS + 1u;
    EXPECT_EQ(feed_all_at(p, frame, n, later, 8), PACKET);
}