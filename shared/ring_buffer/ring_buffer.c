#include "ring_buffer.h"
#include <stddef.h>

_Static_assert((RING_BUFFER_SIZE & (RING_BUFFER_SIZE - 1u)) == 0u, "RING_BUFFER_SIZE must be a power of two");

#define RING_BUFFER_MASK (RING_BUFFER_SIZE - 1u)

void ring_buffer_init(ring_buffer_t *rb)
{
    if (rb == NULL)
        return;
    rb->head = 0;
    rb->tail = 0;
    rb->lost = 0;
}

// producer
bool ring_buffer_producer(ring_buffer_t *rb, uint8_t byte)
{
    uint32_t head = rb->head;
    uint32_t next = (head + 1u) & RING_BUFFER_MASK;
    if (next == rb->tail) {
        rb->lost++;
        return false;
    }
    rb->data[head] = byte;
    rb->head = next;
    return true;
}

// consumer
bool ring_buffer_consumer(ring_buffer_t *rb, uint8_t *out)
{
    uint32_t tail = rb->tail;
    if (tail == rb->head)
        return false;
    *out = rb->data[tail];
    rb->tail = (tail + 1u) & RING_BUFFER_MASK;
    return true;
}