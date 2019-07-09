#include "_pop.h"
#include "_utils.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _POP_HEADER_BYTES (12)
#define _POP_HEADER_MAGIC (50273)
#define _POP_HEADER_VERSION (1)

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

static int8_t _pack_bytess(const uint8_t *s, size_t *i, uint16_t *d) {
        size_t j = (*i) + 2;
        for (; *i < j; ++(*i)) {
                *d |= s[*i];
                if (*i < j - 1)
                        *d <<= 8;
        }
        return 0;
}

static int8_t _pack_bytesl(const uint8_t *s, size_t *i, uint32_t *d) {
        size_t j = (*i) + 4;
        for (; *i < j; ++(*i)) {
                *d |= s[*i];
                if (*i < j - 1)
                        *d <<= 8;
        }
        return 0;
}

int8_t _pop_pkt_init(uint8_t *b, size_t bs, _pop_pkt_t **pop_packet) {
        if (bs < _POP_HEADER_BYTES)
                return -1;
        _pop_pkt_t *packet = malloc(sizeof(_pop_pkt_t));
        if (!packet)
                exit(EXIT_FAILURE);

        packet->header = malloc(sizeof(_pop_pkt_hdr_t));
        if (!packet->header)
                exit(EXIT_FAILURE);

        packet->data = malloc(sizeof(uint8_t) * (bs - _POP_HEADER_BYTES + 1));
        if (!packet->data)
                exit(EXIT_FAILURE);

        size_t k = 0;
        size_t i = 0;
        memset(packet->header, 0, sizeof(_pop_pkt_hdr_t));
        memset(packet->data, 0, sizeof(uint8_t) * (bs - _POP_HEADER_BYTES + 1));

        /* parsing packet header */
        /* magic */
        _pack_bytess(b, &i, &packet->header->magic);
        packet->header->magic = ntohs(packet->header->magic);
        /* discard non pop protocol packets */
        if (packet->header->magic != _POP_HEADER_MAGIC) {
                free(packet->header);
                free(packet->data);
                free(packet);
                return -1;
        }
        /* version */
        packet->header->version = b[i++];
        /* discard wrong versioned pop packet */
        if (packet->header->version != _POP_HEADER_VERSION) {
                free(packet->header);
                free(packet->data);
                free(packet);
                return -1;
        }

        /* command */
        packet->header->command = b[i++];
        /* sequence number */
        _pack_bytesl(b, &i, &packet->header->seqnum);
        packet->header->seqnum = ntohl(packet->header->seqnum);
        /* session id */
        _pack_bytesl(b, &i, &packet->header->sessid);
        packet->header->sessid = ntohl(packet->header->sessid);

        /* data payload */
        for (k = 0; i < bs; ++i, ++k)
                (packet->data)[k] = b[i];
        packet->data[k] = '\0';

        /*
        fprintf(stdout,
                "magic: %u\nversion: %u\ncommand: %u\nseqno: %u\nsessid: %u\n"
                "data:\n%s\n",
                packet->header->magic, packet->header->version,
                packet->header->command, packet->header->seqnum,
                packet->header->sessid, (char *)packet->data);
        */

        *pop_packet = packet;
        return 0;
}

int8_t _pop_pkt_destroy(_pop_pkt_t **pop_packet) {
        if (!pop_packet)
                return -1;

        free((*pop_packet)->header);
        free((*pop_packet)->data);
        free(*pop_packet);
        *pop_packet = NULL;
        return 0;
}

uint8_t _pop_pkt_getdata(_pop_pkt_t *pop_packet, uint8_t **data) {
        if (!pop_packet)
                return -1;
        if (!data)
                return -1;
        *data = pop_packet->data;
        return 0;
}

uint8_t _pop_pkt_hdr_getsessid(_pop_pkt_t *pop_packet, uint32_t *sessid) {
        if (!pop_packet)
                return -1;
        if (!sessid)
                return -1;
        *sessid = pop_packet->header->sessid;
        return 0;
}

uint8_t _pop_pkt_hdr_getcommand(_pop_pkt_t *pop_packet, uint8_t *cmd) {
        if (!pop_packet)
                return -1;
        if (!cmd)
                return -1;
        *cmd = pop_packet->header->command;
        return 0;
}

uint8_t _pop_pkt_hdr_getseqnum(_pop_pkt_t *pop_packet, uint32_t *seqnum) {
        if (!pop_packet)
                return -1;
        if (!seqnum)
                return -1;
        *seqnum = pop_packet->header->seqnum;
        return 0;
}
