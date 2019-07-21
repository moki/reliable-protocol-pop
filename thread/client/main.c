#define _POSIX_C_SOURCE (200809L)
#define _XOPEN_SOURCE (700)

/*
#include "_pop.h"
#include "_server.h"
#include "ptpool.h"
*/

#include "_client.h"
#include "_net.h"
#include "_utils.h"

#include <time.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

#define USAGE                                                                  \
        ("Usage: <host> <dest_port> <source_port> to establish connection\n"   \
         "with host <host> at port <dest_port> from port <source_port>\n"      \
         "where ports are < 65536")

int main(int argc, char **argv) {
        _net_udp_conn_t *dialconn;
        _client_state_t *state;
        uintmax_t source_port;
        uintmax_t dest_port;
        int64_t err;
        fd_set _rfds;
        fd_set rfds;
        _buf_t *b;

        /* parse and validate arguments */
        err = argc != 4 ? -1 : 0;
        _check_err(err, USAGE, _FATAL);

        err = _parse_port(argv[2], &dest_port);
        _check_err(err, "err, _parse_port: failed", _FATAL);
        err = dest_port > 65535 ? -1 : 0;
        _check_err(err, "err, port out of range", _FATAL);

        err = _parse_port(argv[3], &source_port);
        _check_err(err, "err, _parse_port: failed", _FATAL);
        err = source_port > 65535 ? -1 : 0;
        _check_err(err, "err, port out of range", _FATAL);

        srand(time(NULL) ^ source_port);

        /* configure dial connection */
        dialconn = malloc(sizeof(_net_udp_conn_t));
        if (!dialconn)
                exit(EXIT_FAILURE);
        err = _net_udp_conn_setdestaddrstr(dialconn, argv[1]);
        _check_err(err, "_net_udp_conn_setdestaddrstr: failed", _FATAL);
        err = _net_udp_conn_setdestport(dialconn, dest_port);
        _check_err(err, "_net_udp_conn_setdestport: failed", _FATAL);
        dialconn->addr_len = sizeof(struct sockaddr_storage);
        dialconn->srcport = source_port;
        err = _net_udp_dial(dialconn);
        _check_err(err, "_net_udp_dial: failed\n", _FATAL);

        /* initialize file descriptors sets */
        FD_ZERO(&rfds);
        FD_ZERO(&_rfds);
        FD_SET(STDIN_FILENO, &_rfds);
        FD_SET(dialconn->sk, &_rfds);

        /* initialize read buffer */
        b = malloc(sizeof(_buf_t));
        if (!b)
                exit(EXIT_FAILURE);
        b->bs = _UDP_MAX_DATA_PAYLOAD;
        b->b = malloc(sizeof(uint8_t) * b->bs);
        if (!b->b)
                exit(EXIT_FAILURE);
        memset(b->b, 0, b->bs);

        /* initialize client state */
        state = malloc(sizeof(_client_state_t));
        if (!state)
                exit(EXIT_FAILURE);
        state->dialconn = dialconn;
        state->sessid = rand() % (1ul << 32);
        state->seqnum = 0;
        state->_rfds = _rfds;
        state->rfds = rfds;
        state->b = b;

        fprintf(stdout, "Setup connection: %s:%lu, source: %lu port\n", argv[1],
                dest_port, source_port);

        /* client loop */
        err = _client_loop(state);
        _check_err(err, "client_loop: failed", _FATAL);

        /* destruction */
        free(b->b);
        free(b);
        _net_udp_conn_destroy(dialconn);
        free(dialconn);
        free(state);
        return 0;
}
