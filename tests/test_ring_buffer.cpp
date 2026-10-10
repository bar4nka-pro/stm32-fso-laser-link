#include <gtest/gtest.h>
extern "C" {
#include "ring_buffer.h"
}
// ─────────────────────────────────────────────────────────────
//  basic
// ─────────────────────────────────────────────────────────────
TEST(RingBuffer, EmptyAfterInit) {
    ring_buffer_t rb;
    ring_buffer_init(&rb);

    uint8_t out = 0xEE;
    EXPECT_FALSE(ring_buffer_consumer(&rb, &out));
    EXPECT_EQ(out, 0xEE);          // consumer must not touch *out when empty
    EXPECT_EQ(rb.lost, 0u);
}

TEST(RingBuffer, SingleByteRoundTrip) {
    ring_buffer_t rb;
    ring_buffer_init(&rb);

    ASSERT_TRUE(ring_buffer_producer(&rb, 0x42));

    uint8_t out = 0;
    ASSERT_TRUE(ring_buffer_consumer(&rb, &out));
    EXPECT_EQ(out, 0x42);
    EXPECT_FALSE(ring_buffer_consumer(&rb, &out));   // empty again
}

// one slot always stays free -> full means next(head) == tail
static const uint32_t CAPACITY = RING_BUFFER_SIZE - 1u;
// ─────────────────────────────────────────────────────────────
//  FIFO order test
// ─────────────────────────────────────────────────────────────
TEST(RingBuffer, FifoOrder) {
    ring_buffer_t rb;
    ring_buffer_init(&rb);
    const uint8_t in[] = {'a', 'b', 'c', 'd', 'e', 'f'};
    for (uint8_t b : in) {
        ASSERT_TRUE(ring_buffer_producer(&rb, b));
    }
    for (uint8_t expected : in) {
        uint8_t out = 0;
        ASSERT_TRUE(ring_buffer_consumer(&rb, &out));
        EXPECT_EQ(out, expected);
    }
}
// ─────────────────────────────────────────────────────────────
//  run to max cap; lost counter
// ─────────────────────────────────────────────────────────────
TEST(RingBuffer, FullRejectsAndCountsLost) {
    ring_buffer_t rb;
    ring_buffer_init(&rb);

    for (uint32_t i = 0; i < CAPACITY; i++) {
        ASSERT_TRUE(ring_buffer_producer(&rb, (uint8_t)i)) << "i = " << i;
    }
    EXPECT_FALSE(ring_buffer_producer(&rb, 0xFF));   // 256th byte must be rejected
    EXPECT_EQ(rb.lost, 1u);

    for (uint32_t i = 0; i < CAPACITY; i++) {        // rejected byte did not corrupt data
        uint8_t out = 0;
        ASSERT_TRUE(ring_buffer_consumer(&rb, &out));
        EXPECT_EQ(out, (uint8_t)i);
    }
    uint8_t out = 0;
    EXPECT_FALSE(ring_buffer_consumer(&rb, &out));
}
// ─────────────────────────────────────────────────────────────
//  255, not 256 test (head==tail - empty, next(head)==tail - full
// ─────────────────────────────────────────────────────────────
TEST(RingBuffer, WrapAround) {
    ring_buffer_t rb;
    ring_buffer_init(&rb);

    for (uint32_t i = 0; i < 3u * RING_BUFFER_SIZE; i++) {
        ASSERT_TRUE(ring_buffer_producer(&rb, (uint8_t)i)) << "i = " << i;
        uint8_t out = 0;
        ASSERT_TRUE(ring_buffer_consumer(&rb, &out));
        EXPECT_EQ(out, (uint8_t)i);
        ASSERT_LT(rb.head, RING_BUFFER_SIZE);   // indices never leave the array
        ASSERT_LT(rb.tail, RING_BUFFER_SIZE);
    }
}
// ─────────────────────────────────────────────────────────────
//  reading from emprty buffer
// ─────────────────────────────────────────────────────────────
TEST(RingBuffer, ConsumerOnEmptyKeepsState) {
    ring_buffer_t rb;
    ring_buffer_init(&rb);

    ASSERT_TRUE(ring_buffer_producer(&rb, 1));      // move indices away from 0
    uint8_t out = 0;
    ASSERT_TRUE(ring_buffer_consumer(&rb, &out));

    const uint32_t head = rb.head;
    const uint32_t tail = rb.tail;
    EXPECT_FALSE(ring_buffer_consumer(&rb, &out));
    EXPECT_EQ(rb.head, head);
    EXPECT_EQ(rb.tail, tail);
    EXPECT_EQ(rb.lost, 0u);                          // empty read is not a loss
}