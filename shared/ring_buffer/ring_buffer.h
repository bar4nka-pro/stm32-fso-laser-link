#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

#define RING_BUFFER_SIZE 256u

typedef struct ring_buffer {
    volatile uint8_t data[RING_BUFFER_SIZE];
    volatile uint32_t head;
    volatile uint32_t tail;
    volatile uint32_t lost;
} ring_buffer_t;

void ring_buffer_init(ring_buffer_t *rb);
bool ring_buffer_producer(ring_buffer_t *rb, uint8_t byte); // returns false if buffer is full
bool ring_buffer_consumer(ring_buffer_t *rb, uint8_t *out); //returns false if buffer is empty

#endif
