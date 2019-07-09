#pragma once

#include <inttypes.h>
#include <unistd.h>

typedef struct _pop_pkt_hdr _pop_pkt_hdr_t;
typedef struct _pop_pkt _pop_pkt_t;

#define HELLO (0)
#define DATA (1)
#define ALIVE (2)
#define GOODBYE (3)

extern int8_t _pop_pkt_init(uint8_t *b, size_t bs, _pop_pkt_t **pop_packet);
extern int8_t _pop_pkt_destroy(_pop_pkt_t **pop_packet);
extern uint8_t _pop_pkt_getdata(_pop_pkt_t *pop_packet, uint8_t **data);
extern uint8_t _pop_pkt_hdr_getsessid(_pop_pkt_t *pop_packet, uint32_t *sessid);
extern uint8_t _pop_pkt_hdr_getcommand(_pop_pkt_t *pop_packet, uint8_t *cmd);
extern uint8_t _pop_pkt_hdr_getseqnum(_pop_pkt_t *pop_packet, uint32_t *seqnum);
