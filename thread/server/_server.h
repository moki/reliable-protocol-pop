#pragma once

#include <inttypes.h>
#include <pthread.h>

typedef struct _server_session _server_session_t;

struct _server_session {
        _server_session_t *next;
};

typedef struct _server_sessions _server_sessions_t;

struct _server_sessions {
        pthread_mutex_t sessions_lock;
        _server_session_t *head;
};

extern int8_t _server_sessions_init(_server_sessions_t *sessions);
extern int8_t _server_sessions_destroy(_server_sessions_t *sessions);
