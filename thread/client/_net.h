#pragma once
#define _POSIX_C_SOURCE (200809L)
#define _XOPEN_SOURCE (700)

#include "_utils.h"
#include <netdb.h>

#define _UDP_MAX_DATA_PAYLOAD (65507)

struct _net_udp_conn {
        struct sockaddr_storage addr;
        socklen_t addr_len;
        char srcaddrstr[INET6_ADDRSTRLEN];
        char destaddrstr[INET6_ADDRSTRLEN];
        uint16_t destport;
        uint16_t srcport;
        int sk;
};

typedef struct _net_udp_conn _net_udp_conn_t;

extern int8_t _net_udp_listen(_net_udp_conn_t *conn);
extern int8_t _net_udp_dial(_net_udp_conn_t *conn);
extern int8_t _net_udp_read(_net_udp_conn_t *conn, _buf_t *b);
extern int8_t _net_udp_write(_net_udp_conn_t *conn, _buf_t *b);
extern int8_t _net_udp_conn_setsrcaddrstr(_net_udp_conn_t *conn);
extern int8_t _net_udp_conn_setdestaddrstr(_net_udp_conn_t *conn,
                                           char addr[INET6_ADDRSTRLEN]);
extern int8_t _net_udp_conn_setsrcport(_net_udp_conn_t *conn);
extern int8_t _net_udp_conn_setdestport(_net_udp_conn_t *conn, uint16_t port);
