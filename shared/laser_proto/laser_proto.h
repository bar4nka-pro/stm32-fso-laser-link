#ifndef LASER_PROTO_H
#define LASER_PROTO_H

#include <stdint.h>
#include <stddef.h>

/* format : SYNC1 SYNC2 LEN DATA[LEN] CHECKSUM
   CHECKSUM = LEN XOR-(of all bytes) DATA                */

#define SYNC1        0xAAu
#define SYNC2        0x55u
#define MAX_PAYLOAD  64u
#define OVERHEAD     4u   /* SYNC1 + SYNC2 + LEN + CHECKSUM */
#define MAX_FRAME    (MAX_PAYLOAD + OVERHEAD)

/* --- assembly --- */

/* assembling complete shot out.
   returns the frame lenght in bytes, OR 0 IF argum are invalid. */
size_t build_frame(const uint8_t *payload, uint8_t len,
                         uint8_t *out, size_t out_cap);

/* --- disassembly --- */

typedef enum {
    IDLE,      /* byte received / frame not complete */
    PACKET,    /* frame is ready, CHECKSUM OK */
    BAD_CRC,   /* frame is full, BAD CHECKSUM */
    BAD_LEN    /* data length BAD (very bad) */
} result_t;

typedef enum {
    ST_SYNC1,
    ST_SYNC2,
    ST_LENGTH,
    ST_DATA,
    ST_CHECKSUM
} ST_t;

typedef struct {
    ST_t state;
    uint8_t buffer[MAX_PAYLOAD];
    uint8_t length;
    uint8_t index;
    uint8_t checksum;
} parser_t;

void parser_init(parser_t *p);

/* feeds to parser 1 byte, when PACKET —
   returns p->buffer stored in payload, length p->length.
   reading till next call */
result_t feed_byte(parser_t *p, uint8_t byte);

#endif