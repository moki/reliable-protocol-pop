#pragma once

#include "_net.h"
#include "_pop.h"
#include "ptpool.h"
#include <sys/select.h>

typedef struct _server_session_packet _server_session_packet_t;
typedef struct _server_session _server_session_t;
typedef struct _server_sessions _server_sessions_t;
typedef struct server_state server_state_t;

struct _server_sessions {
        pthread_mutex_t lock;
        _server_session_t *head;
};

struct _server_session {
        _server_session_packet_t *head;
        _server_session_packet_t *last;
        _server_session_t *next;
        _net_udp_conn_t *peerconn;
        uint32_t seqnum;
        uint32_t id;
};

struct server_state {
        _server_sessions_t *sessions;
        _net_udp_conn_t *lisconn;
        ptpool_t *thread_pool;
        _buf_t *b;
        fd_set _rfds;
        fd_set rfds;
};


extern int8_t _server_sessions_init(_server_sessions_t *ss);
extern int8_t _server_sessions_destroy(_server_sessions_t *ss);
extern int8_t _server_sessions_addsession(_server_sessions_t *ss,
                                          _server_session_t *s);
extern int8_t _server_sessions_removesession(_server_sessions_t *ss,
                                             _server_session_t *s);
extern int8_t _server_sessions_lookupbyid(_server_sessions_t *ss, uint32_t id,
                                          _server_session_t **s);

extern int8_t _server_session_init(_server_session_t *s);
extern int8_t _server_session_setid(_server_session_t *s, uint32_t id);
extern int8_t _server_session_setseqnum(_server_session_t *s, uint32_t seqnum);
extern int8_t _server_session_setpeerconn(_server_session_t *s,
                                          _net_udp_conn_t *conn);

extern int8_t _server_session_addpacket(_server_session_t *s, _pop_pkt_t *p);
extern int8_t _server_session_removepacket(_server_session_t *s,
                                           _pop_pkt_t **p);
int8_t _server_session_getseqnum(_server_session_t *s, uint32_t *seqnum);
int8_t _server_session_getid(_server_session_t *s, uint32_t *id);
extern int8_t server_loop(server_state_t *state);
