#pragma once

#include "_utils.h"
#include <inttypes.h>
#include <unistd.h>

typedef struct _pop_pkt_hdr _pop_pkt_hdr_t;
typedef struct _pop_pkt _pop_pkt_t;

struct _pop_pkt_hdr {
        uint16_t magic;
        uint8_t version;
        uint8_t command;
        uint32_t seqnum;
        uint32_t sessid;
};

struct _pop_pkt {
        _pop_pkt_hdr_t *header;
        uint8_t *data;
};

#define _POP_HEADER_BYTES (12)
#define _POP_HEADER_MAGIC (50273)
#define _POP_HEADER_VERSION (1)

#define HELLO (0)
#define DATA (1)
#define ALIVE (2)
#define GOODBYE (3)

extern int8_t _pop_pkt_init(uint8_t *b, size_t bs, _pop_pkt_t **pop_packet);
extern int8_t _pop_pkt_destroy(_pop_pkt_t **pop_packet);
extern int8_t _pop_pkt_getdata(_pop_pkt_t *pop_packet, uint8_t **data);
extern int8_t _pop_pkt_hdr_getsessid(_pop_pkt_t *pop_packet, uint32_t *sessid);
extern int8_t _pop_pkt_hdr_getcommand(_pop_pkt_t *pop_packet, uint8_t *cmd);
extern int8_t _pop_pkt_hdr_getseqnum(_pop_pkt_t *pop_packet, uint32_t *seqnum);
extern int8_t _pop_pkt_serialize(_pop_pkt_t *packet, _buf_t *b, size_t ds);
extern int8_t _pop_pkt_setmagic(_pop_pkt_t *packet, uint16_t magic);
extern int8_t _pop_pkt_setversion(_pop_pkt_t *packet, uint8_t version);
extern int8_t _pop_pkt_setcommand(_pop_pkt_t *packet, uint8_t command);
extern int8_t _pop_pkt_setseqnum(_pop_pkt_t *packet, uint32_t seqnum);
extern int8_t _pop_pkt_setsessid(_pop_pkt_t *packet, uint32_t sessid);
extern int8_t _pop_pkt_setdata(_pop_pkt_t *pop_packet, uint8_t *data);
