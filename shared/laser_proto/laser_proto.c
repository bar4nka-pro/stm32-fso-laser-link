#include "laser_proto.h"

size_t build_frame(const uint8_t *payload, uint8_t len,
                         uint8_t *out, size_t out_cap)
{
    if (payload == NULL || out == NULL)
        return 0;
    if (len == 0 || len > MAX_PAYLOAD)
        return 0;
    if (out_cap < (size_t)len + OVERHEAD)
        return 0;

    uint8_t checksum = len;
    size_t  n = 0;

    out[n++] = SYNC1;
    out[n++] = SYNC2;
    out[n++] = len;

    for (uint8_t i = 0; i < len; i++) {
        out[n++] = payload[i];
        checksum ^= payload[i];
    }

    out[n++] = checksum;
    return n;
}
void  parser_init(parser_t *p) {
    if (p == NULL) return;
    p->state    = ST_SYNC1;
    p->length   = 0;
    p->index    = 0;
    p->checksum = 0;
}

result_t feed_byte(parser_t *p, uint8_t byte)
{
    switch (p->state) {

        case ST_SYNC1:
            if (byte == SYNC1)
                p->state = ST_SYNC2;
            return IDLE;

        case ST_SYNC2:
            if (byte == SYNC2)
                p->state = ST_LENGTH;
            else if (byte != SYNC1)
                p->state = ST_SYNC1;
            /* if SYNC1 MCU stays here AA AA 55 — valid opening */
            return IDLE;

        case ST_LENGTH:
            if (byte == 0 || byte > MAX_PAYLOAD) {
                p->state = ST_SYNC1;
                return BAD_LEN;
            }
            p->length   = byte;
            p->index    = 0;
            p->checksum = byte;
            p->state    = ST_DATA;
            return IDLE;

        case ST_DATA:
            p->buffer[p->index++] = byte;
            p->checksum ^= byte;
            if (p->index >= p->length)
                p->state = ST_CHECKSUM;
            return IDLE;

        case ST_CHECKSUM:
            p->state = ST_SYNC1; /* waiting for new frame whatever happens */
            return (byte == p->checksum) ? PACKET : BAD_CRC;

        default:
            p->state = ST_SYNC1;
            return IDLE;
    }
}