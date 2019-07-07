#pragma once

#include "_net.h"
#include "ptpool.h"
#include <sys/select.h>

typedef struct _server_session _server_session_t;

struct _server_session {
        _server_session_t *next;
};

typedef struct _server_sessions _server_sessions_t;

struct _server_sessions {
        pthread_mutex_t lock;
        _server_session_t *head;
};

typedef struct server_state server_state_t;

struct server_state {
        _server_sessions_t *sessions;
        _net_udp_conn_t *lisconn;
        ptpool_t *thread_pool;
        _buf_t *b;
        fd_set _rfds;
        fd_set rfds;
};

extern int8_t _server_sessions_init(_server_sessions_t *sessions);
extern int8_t _server_sessions_add(_server_sessions_t *sessions,
                                   _server_session_t *session);
extern int8_t _server_sessions_destroy(_server_sessions_t *sessions);

extern int8_t server_loop(server_state_t *state);
