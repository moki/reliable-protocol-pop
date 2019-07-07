#include "_server.h"
#include "_pop.h"
#include "_utils.h"
#include "ptpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct peer_state {
        _pop_pkt_t *pop_packet;
        _server_sessions_t *sessions;
        char sender_addr[INET6_ADDRSTRLEN];
};

typedef struct peer_state peer_state_t;

int8_t _server_sessions_init(_server_sessions_t *sessions) {
        if (!sessions)
                return -1;
        if (pthread_mutex_init(&(sessions->lock), NULL) < 0)
                return -1;
        return 0;
}

int8_t _server_sessions_destroy(_server_sessions_t *sessions) {
        if (!sessions)
                return -1;
        /* TODO:
         * acquire mutex, and iterate over all sessions
         * terminating and cleaning.
         */
        if (pthread_mutex_destroy(&(sessions->lock)) < 0)
                return -1;
        return 0;
}

void _server_peer_process(void *arg) {
        if (!arg)
                exit(EXIT_FAILURE);
        peer_state_t *peer_state = (peer_state_t *)arg;
        int64_t err;
        fprintf(stdout, "@thread from: %s\ndata: %s\n", peer_state->sender_addr,
                _pop_pkt_getdata(peer_state->pop_packet));
        err = _pop_pkt_destroy(&(peer_state->pop_packet));
        _check_err(err, "_pop_pkt_destroy: failed", _FATAL);
        free(peer_state);
}

int8_t server_loop(server_state_t *state) {
        int64_t err;

        for (;;) {
                _pop_pkt_t *pop_packet;
                peer_state_t *peer_state;

                state->rfds = state->_rfds;
                memset(state->b->b, 0, state->b->bs);
                err = select(state->lisconn->sk + 1, &state->rfds, NULL, NULL,
                             NULL);
                _check_err(err, "select: failed", _FATAL);

                /* handle "q", eof */
                if (FD_ISSET(STDIN_FILENO, &state->rfds)) {
                        state->b->bwr =
                                read(STDIN_FILENO, state->b->b, state->b->bs);
                        _check_err(state->b->bwr, "read: failed", _FATAL);
                        if (state->b->b[0] == 'q' || !(state->b->bwr))
                                return 0;
                        continue;
                }

                /* handle inc. connection */
                err = _net_udp_read(state->lisconn, state->b);
                _check_err(err, "_net_udp_read: failed", _FATAL);
                err = _net_udp_conn_getaddr(state->lisconn);
                _check_err(err, "_net_udp_conn_getaddr: failed", _FATAL);
                err = _pop_pkt_init(state->b->b, state->b->bs, &pop_packet);
                _check_err(err, "_pop_pkt_init: packet discarded", !_FATAL);
                if (err)
                        continue;
                /* initialize peer state */
                peer_state = malloc(sizeof(peer_state_t));
                if (!peer_state)
                        exit(EXIT_FAILURE);
                strncpy(peer_state->sender_addr, state->lisconn->addrstr,
                        INET6_ADDRSTRLEN);
                peer_state->pop_packet = pop_packet;
                peer_state->sessions = state->sessions;
                ptpool_wqueue_add(state->thread_pool, _server_peer_process,
                                  peer_state);
        }

        return 0;
}
