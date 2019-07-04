#pragma once
#define _POSIX_C_SOURCE (200809L)
#define _XOPEN_SOURCE (700)

#include <inttypes.h>
#include <unistd.h>

#define _UDP_MAX_DATA_PAYLOAD (65507)

extern int8_t _net_listen_udp(char *port, int *sockfd);
extern int8_t _net_read_udp(int sockfd, uint8_t *b, size_t bs, ssize_t *read);
