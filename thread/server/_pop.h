#pragma once

#include <inttypes.h>
#include <stdlib.h>

typedef struct _pop_pkt_hdr _pop_pkt_hdr_t;
typedef struct _pop_pkt _pop_pkt_t;

extern int8_t _pop_pkt_init(uint8_t *b, size_t bs, _pop_pkt_t **pop_packet);
extern int8_t _pop_pkt_destroy(_pop_pkt_t **pop_packet);
