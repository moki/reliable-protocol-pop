#pragma once
#define _POSIX_C_SOURCE (200809L)
#define _XOPEN_SOURCE (700)

#include <inttypes.h>

extern int8_t _net_listen_udp(char *port, int *listener);
