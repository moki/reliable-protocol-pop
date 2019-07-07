#pragma once
#define _POSIX_C_SOURCE (200809L)
#define _XOPEN_SOURCE (700)

#include "_utils.h"
#include <netdb.h>

#define _UDP_MAX_DATA_PAYLOAD (65507)

struct _net_udp_conn {
        struct sockaddr_storage addr;
        socklen_t addr_len;
        char addrstr[INET6_ADDRSTRLEN];
        char *port;
        int sk;
};

typedef struct _net_udp_conn _net_udp_conn_t;

extern int8_t _net_udp_listen(_net_udp_conn_t *conn);
extern int8_t _net_udp_dial(_net_udp_conn_t *conn);
extern int8_t _net_udp_read(_net_udp_conn_t *conn, _buf_t *b);
extern int8_t _net_udp_conn_getaddr(_net_udp_conn_t *conn);
