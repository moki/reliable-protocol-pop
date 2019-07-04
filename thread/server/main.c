#define _POSIX_C_SOURCE (200809L)
#define _XOPEN_SOURCE (700)

#include "_net.h"
#include "_pop.h"
#include "_utils.h"
#include "ptpool.h"

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USAGE ("Usage: server port\nWhere port is < 65536")

int8_t serve_peer(int udpsock) {
        _pop_pkt_t *pop_packet;
        uint8_t *udp_buffer;
        ssize_t udpread;
        size_t buffersize;
        int8_t err;

        buffersize = _UDP_MAX_DATA_PAYLOAD + 1;
        udp_buffer = malloc(buffersize);
        if (!udp_buffer)
                exit(EXIT_FAILURE);

        memset(udp_buffer, 0, buffersize);
        err = _net_read_udp(udpsock, udp_buffer, buffersize, &udpread);
        _check_err(err, "_net_read_udp: failed", _FATAL);

        err = _pop_pkt_init(udp_buffer, udpread, &pop_packet);
        _check_err(err, "_pop_pkt_init: reject not a pop packet", !_FATAL);
        if (err) {
                free(udp_buffer);
                return 0;
        }

        err = _pop_pkt_destroy(&pop_packet);
        _check_err(err, "_pop_pkt_destroy: failed", _FATAL);

        free(udp_buffer);
        return 0;
}

int8_t server_loop(int listener) {
        size_t buffersize = _UDP_MAX_DATA_PAYLOAD + 1;
        uint8_t *udp_buffer = malloc(buffersize);
        if (!udp_buffer)
                exit(EXIT_FAILURE);
        memset(udp_buffer, 0, buffersize);

        ssize_t udpread;
        int8_t err;

        for (;;) {
                udpread = recv(listener, udp_buffer, buffersize, MSG_PEEK);
                err = udpread < 0 ? -1 : 0;
                _check_err(err, "recv: failed", _FATAL);
                if (strcmp((const char *)udp_buffer, "q") == 0) {
                        free(udp_buffer);
                        return 0;
                }
                err = serve_peer(listener);
                _check_err(err, "server_peer: failed", _FATAL);
        }

        free(udp_buffer);
        return 0;
}

int main(int argc, char **argv) {
        uintmax_t portnum;
        int8_t err;
        int listener;
        ptpool_attr_t ptpattr;
        ptpool_t *thread_pool;

        err = argc != 2 ? -1 : 0;
        _check_err(err, USAGE, _FATAL);

        err = _parse_port(argv[1], &portnum);
        _check_err(err, "err, _parse_port: failed", _FATAL);

        err = portnum > 65535 ? -1 : 0;
        _check_err(err, "err, port out of range", _FATAL);

        err = ptpool_attr_init(&ptpattr);
        _check_err(err, "ptpool_attr_init: failed", _FATAL);
        err = ptpool_attr_setpoolsize(&ptpattr, 4);
        _check_err(err, "ptpool_attr_setpoolsize: failed", _FATAL);
        err = ptpool_attr_setqueuesize(&ptpattr, 12);
        _check_err(err, "ptpool_attr_setqueuesize: failed", _FATAL);

        thread_pool = malloc(sizeof(ptpool_t));
        if (!thread_pool)
                exit(EXIT_FAILURE);
        err = ptpool_init(thread_pool, &ptpattr);
        _check_err(err, "ptpool_init: failed", _FATAL);

        err = _net_listen_udp(argv[1], &listener);
        _check_err(err, "_net_listen_udp: failed", _FATAL);

        fprintf(stdout, "server listens on localhost:%s\n", argv[1]);
        err = server_loop(listener);
        _check_err(err, "server_loop: failed", _FATAL);

        err = ptpool_destroy(thread_pool, true);
        _check_err(err, "ptpool_destroy: failed", _FATAL);

        return 0;
}
