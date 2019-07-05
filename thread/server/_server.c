#include "_server.h"
#include "_utils.h"
#include <pthread.h>

int8_t _server_sessions_init(_server_sessions_t *sessions) {
        if (!sessions)
                return -1;
        if (pthread_mutex_init(&(sessions->sessions_lock), NULL) < 0)
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
        if (pthread_mutex_destroy(&(sessions->sessions_lock)) < 0)
                return -1;
        return 0;
}
