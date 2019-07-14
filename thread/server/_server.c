#define _GNU_SOURCE (1)
#define _POSIX_C_SOURCE (200809L)
#define _XOPEN_SOURCE (700)
#include "_server.h"
#include "_pop.h"
#include "_utils.h"
#include "ptpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>

struct process_args {
        _server_sessions_t *ss;
};

typedef struct process_args process_args_t;

struct distribute_args {
        _server_sessions_t *ss;
        _pop_pkt_t *p;
        char address[INET6_ADDRSTRLEN];
        uint16_t port;
};

typedef struct distribute_args distribute_args_t;

struct _server_session_packet {
        _server_session_packet_t *next;
        _pop_pkt_t *packet;
};

int8_t _server_sessions_init(_server_sessions_t *ss) {
        if (!ss)
                return -1;
        ss->head = NULL;
        if (pthread_mutex_init(&(ss->lock), NULL) < 0)
                return -1;
        return 0;
}

int8_t _server_session_init(_server_session_t *s) {
        if (!s)
                return -1;
        s->head = s->last = NULL;
        s->next = NULL;
        return 0;
}

int8_t _server_session_setid(_server_session_t *s, uint32_t id) {
        if (!s)
                return -1;
        s->id = id;
        return 0;
}

int8_t _server_session_setseqnum(_server_session_t *s, uint32_t seqnum) {
        if (!s)
                return -1;
        s->seqnum = seqnum;
        return 0;
}

int8_t _server_session_setpeerconn(_server_session_t *s,
                                   _net_udp_conn_t *conn) {
        if (!s)
                return -1;
        if (!conn)
                return -1;
        s->peerconn = conn;
        return 0;
}

int8_t _server_session_addpacket(_server_session_t *s, _pop_pkt_t *p) {
        if (!s)
                return -1;
        if (!p)
                return -1;

        _server_session_packet_t *packet =
                malloc(sizeof(_server_session_packet_t));
        if (!packet)
                exit(EXIT_FAILURE);
        packet->packet = p;
        packet->next = NULL;

        if (!(s->head)) {
                s->head = s->last = packet;
        } else {
                s->last->next = packet;
                s->last = packet;
        }
        return 0;
}

int8_t _server_session_removepacket(_server_session_t *s, _pop_pkt_t **p) {
        if (!s)
                return -1;
        if (!p)
                return -1;
        _server_session_packet_t *head = s->head;
        *p = head->packet;
        if (s->head == s->last)
                s->head = s->last = NULL;
        else
                s->head = s->head->next;
        free(head);
        return 0;
}

int8_t _server_sessions_addsession(_server_sessions_t *ss,
                                   _server_session_t *s) {
        if (!ss)
                return -1;
        if (!s)
                return -1;
        s->next = ss->head;
        ss->head = s;
        return 0;
}

int8_t _server_sessions_removesession(_server_sessions_t *ss,
                                      _server_session_t *s) {
        if (!ss)
                return -1;
        if (!s)
                return -1;
        _server_session_t *cursor;
        _server_session_t *_cursor;
        for (cursor = ss->head; cursor; cursor = cursor->next) {
                if (cursor == s)
                        break;
                _cursor = cursor;
        }

        if (!cursor)
                return -1;

        if (cursor == ss->head)
                ss->head = cursor->next;
        else
                _cursor->next = s->next;

        close(s->peerconn->sk);
        free(s->peerconn);
        free(s);

        return 0;
}

int8_t _server_sessions_lookupbyid(_server_sessions_t *ss, uint32_t id,
                                   _server_session_t **s) {
        if (!ss)
                return -1;
        if (!s)
                return -1;
        _server_session_t *cursor;
        for (cursor = ss->head; cursor; cursor = cursor->next) {
                if (cursor->id == id)
                        break;
        }

        *s = cursor;

        return 0;
}

int8_t _server_sessions_destroy(_server_sessions_t *ss) {
        if (!ss)
                return -1;
        _server_session_t *cursor;
        int8_t err;
        err = pthread_mutex_lock(&(ss->lock)) ? -1 : 0;
        _check_err(err, "pthread_mutex_lock: failed", _FATAL);
        for (cursor = ss->head; cursor;) {
                ss->head = cursor;
                cursor = cursor->next;
                close(ss->head->peerconn->sk);
                free(ss->head->peerconn);
                free(ss->head);
        }
        err = pthread_mutex_unlock(&(ss->lock)) ? -1 : 0;
        _check_err(err, "pthread_mutex_unlock", _FATAL);
        if (pthread_mutex_destroy(&(ss->lock)) < 0)
                return -1;
        return 0;
}

void _server_session_process(void *args) {
        if (!args)
                exit(EXIT_FAILURE);
        _server_sessions_t *ss = ((process_args_t *)args)->ss;
        if (!ss)
                exit(EXIT_FAILURE);
        int8_t err;
        err = pthread_mutex_lock(&(ss->lock));
        _check_err(err, "pthread_mutex_lock: failed", _FATAL);
        _pop_pkt_t *p;
        uint32_t seqnum;
        uint32_t sessid;
        uint8_t cmd;

        /* debug */
        fprintf(stdout, "%lu\n\n", syscall(SYS_gettid));

        _server_session_t *cursor;
        for (cursor = ss->head; cursor; cursor = cursor->next) {
                if (cursor->head)
                        break;
        }

        /* all packets processed */
        if (!cursor)
                return;

        err = _server_session_removepacket(cursor, &p);
        _check_err(err, "_server_session_removepacket: failed", _FATAL);
        err = _pop_pkt_hdr_getcommand(p, &cmd);
        _check_err(err, "_pop_pkt_hdr_getcommand: failed", _FATAL);
        err = _pop_pkt_hdr_getseqnum(p, &seqnum);
        _check_err(err, "_pop_pkt_hdr_getseqnum: failed", _FATAL);
        err = _pop_pkt_hdr_getsessid(p, &sessid);
        _check_err(err, "_pop_pkt_hdr_getsessid: failed", _FATAL);

        if (cmd == HELLO && seqnum == 0 && seqnum == cursor->seqnum) {
                fprintf(stdout, "Hello packet\n");
                _buf_t *bf = malloc(sizeof(_buf_t));
                bf->bs = strlen("HELLO") + 1;
                bf->b = malloc(sizeof(uint8_t) * bf->bs);
                if (!bf->b)
                        exit(EXIT_FAILURE);
                memset(bf->b, 0, bf->bs);
                strncpy((char *)bf->b, "HELLO", strlen("HELLO"));

                err = _server_session_setid(cursor, sessid);
                _check_err(err, "_server_session_setid: failed", _FATAL);

                err = _net_udp_write(cursor->peerconn, bf);
                _check_err(err, "_net_udp_write: failed", _FATAL);

                err = _server_session_setseqnum(cursor, cursor->seqnum + 1);
                _check_err(err, "_server_session_setseqnum: failed", _FATAL);

                free(bf->b);
                free(bf);
        } else if (cmd == DATA && seqnum == cursor->seqnum) {
                fprintf(stdout, "Data packet\n");
                _buf_t *bf = malloc(sizeof(_buf_t));
                bf->bs = strlen("ALIVE") + 1;

                bf->b = malloc(sizeof(uint8_t) * bf->bs);
                if (!bf->b)
                        exit(EXIT_FAILURE);
                memset(bf->b, 0, bf->bs);
                strncpy((char *)bf->b, "ALIVE", strlen("ALIVE"));

                err = _net_udp_write(cursor->peerconn, bf);
                _check_err(err, "_net_udp_write: failed", _FATAL);

                err = _server_session_setseqnum(cursor, cursor->seqnum + 1);
                _check_err(err, "_server_session_setseqnum: failed", _FATAL);

                free(bf->b);
                free(bf);
        }

        err = _pop_pkt_destroy(&p);
        _check_err(err, "_pop_pkt_destroy: failed", _FATAL);

        err = pthread_mutex_unlock(&(ss->lock));
        _check_err(err, "pthread_mutex_unlock: failed", _FATAL);
        free(args);
}

void _server_packet_distribute(void *args) {
        if (!args)
                exit(EXIT_FAILURE);
        distribute_args_t *passed = (distribute_args_t *)args;
        _server_sessions_t *ss = passed->ss;
        if (!ss)
                exit(EXIT_FAILURE);
        _pop_pkt_t *p = passed->p;
        if (!p)
                exit(EXIT_FAILURE);
        char *address = passed->address;
        if (!address)
                exit(EXIT_FAILURE);
        uint16_t port = passed->port;
        if (!port)
                exit(EXIT_FAILURE);

        _server_session_t *s;
        uint32_t psid;
        int8_t err;

        err = pthread_mutex_lock(&(ss->lock));
        _check_err(err, "pthread_mutex_lock", _FATAL);

        /* debug */
        fprintf(stdout, "%lu\n\n", syscall(SYS_gettid));

        err = _pop_pkt_hdr_getsessid(p, &psid);
        _check_err(err, "_pop_pkt_hdr_getsessid: failed", _FATAL);
        err = _server_sessions_lookupbyid(ss, psid, &s);
        _check_err(err, "_server_sessions_lookupbyid: failed", _FATAL);

        // create new one if session not present
        if (!s) {
                _net_udp_conn_t *peerconn = malloc(sizeof(_net_udp_conn_t));
                if (!peerconn)
                        exit(EXIT_FAILURE);
                err = _net_udp_conn_setdestport(peerconn, port);
                _check_err(err, "_net_udp_conn_setdestport: failed", _FATAL);
                err = _net_udp_conn_setdestaddrstr(peerconn, address);
                _check_err(err, "_net_udp_conn_setdestaddrstr: failed", _FATAL);
                err = _net_udp_dial(peerconn);
                _check_err(err, "_net_udp_dial: failed", _FATAL);

                s = malloc(sizeof(_server_session_t));
                if (!s)
                        exit(EXIT_FAILURE);
                err = _server_session_init(s);
                _check_err(err, "_server_session_init: failed", _FATAL);
                err = _server_session_setid(s, psid);
                _check_err(err, "_server_session_init: failed", _FATAL);
                err = _server_session_setpeerconn(s, peerconn);
                _check_err(err, "_server_session_setpeerconn: failed", _FATAL);
                err = _server_session_setseqnum(s, 0);
                _check_err(err, "_server_session_setseqnum: failed", _FATAL);
                err = _server_sessions_addsession(ss, s);
                _check_err(err, "_server_sessions_addsession: failed", _FATAL);
        }

        /* append packet to a session's packets queue */
        err = _server_session_addpacket(s, p);
        _check_err(err, "_server_session_setpacket: failed", _FATAL);
        err = pthread_mutex_unlock(&(ss->lock));
        _check_err(err, "pthread_mutex_unlock: failed", _FATAL);
        free(passed);
}

int8_t server_loop(server_state_t *state) {
        distribute_args_t *distribute_args;
        process_args_t *process_args;
        _pop_pkt_t *pop_packet;
        int64_t err;

        for (;;) {
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

                /* debug */
                fprintf(stdout, "%lu\n\n", syscall(SYS_gettid));

                /* handle inc. connection */
                err = _net_udp_read(state->lisconn, state->b);
                _check_err(err, "_net_udp_read: failed", _FATAL);
                err = _net_udp_conn_setsrcaddrstr(state->lisconn);
                _check_err(err, "_net_udp_conn_getaddr: failed", _FATAL);
                err = _net_udp_conn_setsrcport(state->lisconn);
                _check_err(err, "_net_udp_conn_setport: failed", _FATAL);
                /* parse packet */
                err = _pop_pkt_init(state->b->b, state->b->bs, &pop_packet);
                _check_err(err, "_pop_pkt_init: packet discarded", !_FATAL);
                if (err)
                        continue;

                /* assign packet to session's packets queue */
                distribute_args = malloc(sizeof(distribute_args_t));
                if (!distribute_args)
                        exit(EXIT_FAILURE);
                distribute_args->ss = state->sessions;
                distribute_args->p = pop_packet;
                distribute_args->port = state->lisconn->srcport;
                if (!strncpy(distribute_args->address,
                             state->lisconn->srcaddrstr, INET6_ADDRSTRLEN)) {
                        exit(EXIT_FAILURE);
                }

                _server_packet_distribute(distribute_args);

                /* run session's packets processing worker */
                process_args = malloc(sizeof(process_args_t));
                if (!process_args)
                        exit(EXIT_FAILURE);
                process_args->ss = state->sessions;

                ptpool_wqueue_add(state->thread_pool, _server_session_process,
                                  process_args);
        }

        return 0;
}
