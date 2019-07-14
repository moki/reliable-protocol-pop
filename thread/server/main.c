#define _POSIX_C_SOURCE (200809L)
#define _XOPEN_SOURCE (700)

#include "_net.h"
#include "_pop.h"
#include "_server.h"
#include "_utils.h"
#include "ptpool.h"
#include <fcntl.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USAGE ("Usage: server port\nWhere port is < 65536")

int main(int argc, char **argv) {
        _server_sessions_t *sessions;
        _net_udp_conn_t *lisconn;
        server_state_t *state;
        ptpool_attr_t ptpattr;
        ptpool_t *thread_pool;
        uintmax_t portnum;
        int64_t err;
        _buf_t *b;
        fd_set _rfds;
        fd_set rfds;
        int flags;

        /* parse and validate port argument */
        err = argc != 2 ? -1 : 0;
        _check_err(err, USAGE, _FATAL);
        err = _parse_port(argv[1], &portnum);
        _check_err(err, "err, _parse_port: failed", _FATAL);
        err = portnum > 65535 ? -1 : 0;
        _check_err(err, "err, port out of range", _FATAL);

        /* configure and initialize thread pool */
        err = ptpool_attr_init(&ptpattr);
        _check_err(err, "ptpool_attr_init: failed", _FATAL);
        err = ptpool_attr_setpoolsize(&ptpattr, 4);
        _check_err(err, "ptpool_attr_setpoolsize: failed", _FATAL);
        err = ptpool_attr_setqueuesize(&ptpattr, 10);
        _check_err(err, "ptpool_attr_setqueuesize: failed", _FATAL);
        thread_pool = malloc(sizeof(ptpool_t));
        if (!thread_pool)
                exit(EXIT_FAILURE);
        err = ptpool_init(thread_pool, &ptpattr);
        _check_err(err, "ptpool_init: failed", _FATAL);

        /* initialize sessions */
        sessions = malloc(sizeof(_server_sessions_t));
        if (!sessions)
                exit(EXIT_FAILURE);
        err = _server_sessions_init(sessions);
        _check_err(err, "_server_sessions_init: failed", _FATAL);

        /* configure listen conn */
        lisconn = malloc(sizeof(_net_udp_conn_t));
        if (!lisconn)
                exit(EXIT_FAILURE);
        lisconn->destport = portnum;
        lisconn->addr_len = sizeof(struct sockaddr_storage);
        err = _net_udp_listen(lisconn);
        _check_err(err, "_net_udp_listen: failed", _FATAL);
        flags = fcntl(lisconn->sk, F_GETFL, 0);
        _check_err(flags, "fcntl: failed", _FATAL);
        err = fcntl(lisconn->sk, F_SETFL, flags | O_NONBLOCK);
        _check_err(err, "fcntl: failed", _FATAL);

        /* initialize file descriptors */
        FD_ZERO(&rfds);
        FD_ZERO(&_rfds);
        FD_SET(STDIN_FILENO, &_rfds);
        FD_SET(lisconn->sk, &_rfds);

        /* initialize read buffer */
        b = malloc(sizeof(_buf_t));
        if (!b)
                exit(EXIT_FAILURE);
        b->bs = _UDP_MAX_DATA_PAYLOAD + 1;
        b->b = malloc(sizeof(uint8_t) * b->bs);
        if (!b->b)
                exit(EXIT_FAILURE);
        memset(b->b, 0, b->bs);

        /* initialize global state */
        state = malloc(sizeof(server_state_t));
        if (!state)
                exit(EXIT_FAILURE);
        state->thread_pool = thread_pool;
        state->sessions = sessions;
        state->lisconn = lisconn;
        state->_rfds = _rfds;
        state->rfds = rfds;
        state->b = b;

        /* step into server loop */
        fprintf(stdout, "server listens on localhost:%u\n", lisconn->destport);
        err = server_loop(state);
        _check_err(err, "server_loop: failed", _FATAL);

        /* TODO: clean-up exising sessions */
        /* clean up */
        err = _server_sessions_destroy(sessions);
        _check_err(err, "_server_session_destroy: failed", _FATAL);
        free(b->b);
        free(b);
        free(lisconn);
        free(sessions);
        free(state);
        err = ptpool_destroy(thread_pool, true);
        _check_err(err, "ptpool_destroy: failed", _FATAL);
        return 0;
}
