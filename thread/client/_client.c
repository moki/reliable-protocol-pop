#include "_client.h"
#include "_pop.h"
#include <stdio.h>
#include <stdlib.h>

#define _FSM_INITIAL (0)
#define _FSM_HELLO_WAIT (0)
#define _FSM_READY (1)
#define _FSM_CLOSING (2)

struct _packet_send_args {
        _net_udp_conn_t *dialconn;
        uint32_t *seqnum;
        uint32_t sessid;
        uint8_t command;
        uint8_t *data;
        size_t ds;
};

typedef struct _packet_send_args _packet_send_args_t;

int8_t _client_packet_send(_packet_send_args_t *args) {
        uint8_t command = args->command;
        uint8_t *data = args->data;

        if (command != HELLO && command != GOODBYE && command != DATA)
                return -1;
        if (!data && command == DATA)
                return -1;

        _net_udp_conn_t *dialconn = args->dialconn;
        uint32_t *seqnum = args->seqnum;
        uint32_t sessid = args->sessid;
        size_t ds = args->ds;
        int64_t err;

        _buf_t *b = malloc(sizeof(_buf_t));
        if (!b)
                exit(EXIT_FAILURE);
        _pop_pkt_t *p = malloc(sizeof(_pop_pkt_t));
        if (!p)
                exit(EXIT_FAILURE);
        p->header = malloc(sizeof(_pop_pkt_hdr_t));
        if (!p->header)
                exit(EXIT_FAILURE);
        err = _pop_pkt_setmagic(p, _POP_HEADER_MAGIC);
        _check_err(err, "_pop_pkt_setmagic: failed", _FATAL);
        err = _pop_pkt_setversion(p, _POP_HEADER_VERSION);
        _check_err(err, "_pop_pkt_setversion: failed", _FATAL);
        err = _pop_pkt_setseqnum(p, *seqnum);
        _check_err(err, "_pop_pkt_set_seqnum: failed", _FATAL);
        err = _pop_pkt_setsessid(p, sessid);
        _check_err(err, "_pop_pkt_setsessid: failed", _FATAL);
        err = _pop_pkt_setcommand(p, command);
        _check_err(err, "_pop_pkt_setcommand: failed", _FATAL);
        if (command == DATA) {
                err = _pop_pkt_setdata(p, data);
                _check_err(err, "_pop_pkt_setdata: failed", _FATAL);
        }

        err = _pop_pkt_serialize(p, b, command == DATA ? ds : 0);
        _check_err(err, "_pop_pkt_serialize: failed", _FATAL);
        err = _net_udp_write(dialconn, b);
        _check_err(err, "_net_udp_write: failed", _FATAL);

        *seqnum = *seqnum + 1;

        free(data);
        free(p->header);
        free(p);
        free(b->b);
        free(b);

        return 0;
}

int8_t _client_loop(_client_state_t *state) {
        _pop_pkt_t *p;
        uint8_t fsmstate;
        int64_t err;
        bool waitsack;

        _net_udp_conn_t *dialconn = state->dialconn;
        uint32_t sessid = state->sessid;
        uint32_t *seqnum = &state->seqnum;
        _buf_t *b = state->b;
        fd_set _rfds = state->_rfds;
        fd_set rfds = state->rfds;
        waitsack = false;

        /* Handshake */
        /* ack */
        _packet_send_args_t *packet_send_args =
                malloc(sizeof(_packet_send_args_t));
        if (!packet_send_args)
                exit(EXIT_FAILURE);
        packet_send_args->dialconn = dialconn;
        packet_send_args->seqnum = seqnum;
        packet_send_args->sessid = sessid;
        packet_send_args->command = HELLO;
        packet_send_args->data = NULL;
        packet_send_args->ds = 0;

        err = _client_packet_send(packet_send_args);
        _check_err(err, "_client_packet_send: failed", _FATAL);
        free(packet_send_args);
        fsmstate = _FSM_HELLO_WAIT;

        /* Ready State */
        for (;;) {
                rfds = _rfds;
                memset(b->b, 0, b->bs);
                err = select(dialconn->sk + 1, &rfds, NULL, NULL, NULL);
                _check_err(err, "select: failed", _FATAL);

                if (fsmstate == _FSM_HELLO_WAIT) {
                        err = _net_udp_read(dialconn, b);
                        _check_err(err, "_net_udp_read: failed", _FATAL);
                        err = _pop_pkt_init(b->b, b->bs, &p);
                        _check_err(err, "_pop_pkt_init: failed", _FATAL);
                        if (p->header->command == HELLO) {
                                fsmstate = _FSM_READY;
                                err = _pop_pkt_destroy(&p);
                                _check_err(err, "_pop_pkt_destroy: failed",
                                           _FATAL);
                        } else if (p->header->command == GOODBYE) {
                                err = _pop_pkt_destroy(&p);
                                _check_err(err, "_pop_pkt_destroy: failed",
                                           _FATAL);
                                return 0;
                        }
                        continue;
                } else if (fsmstate == _FSM_CLOSING) {
                        err = _net_udp_read(dialconn, b);
                        _check_err(err, "_net_udp_read: failed", _FATAL);
                        err = _pop_pkt_init(b->b, b->bs, &p);
                        _check_err(err, "_pop_pkt_init: failed", _FATAL);
                        if (p->header->command == GOODBYE) {
                                err = _pop_pkt_destroy(&p);
                                _check_err(err, "_pop_pkt_destroy: failed",
                                           _FATAL);

                                return 0;
                        }
                        continue;
                } else if (fsmstate == _FSM_READY) {
                        if (waitsack) {
                                err = _net_udp_read(dialconn, b);
                                _check_err(err, "_net_udp_read: failed",
                                           _FATAL);
                                err = _pop_pkt_init(b->b, b->bs, &p);
                                _check_err(err, "_pop_pkt_init: failed",
                                           _FATAL);
                                if (p->header->command == ALIVE) {
                                        waitsack = !waitsack;
                                        err = _pop_pkt_destroy(&p);
                                        _check_err(err,
                                                   "_pop_pkt_destroy: failed",
                                                   _FATAL);
                                } else if (p->header->command == GOODBYE) {
                                        err = _pop_pkt_destroy(&p);
                                        _check_err(err,
                                                   "_pop_pkt_destroy: failed",
                                                   _FATAL);
                                        return 0;
                                } else
                                        exit(EXIT_FAILURE);
                        }
                }

                if (FD_ISSET(STDIN_FILENO, &rfds)) {
                        b->bwr = read(STDIN_FILENO, b->b,
                                      b->bs - _POP_HEADER_BYTES);
                        _check_err(b->bwr, "stdin_read: failed", _FATAL);

                        _packet_send_args_t *packet_send_args =
                                malloc(sizeof(_packet_send_args_t));
                        if (!packet_send_args)
                                exit(EXIT_FAILURE);
                        packet_send_args->dialconn = dialconn;
                        packet_send_args->seqnum = seqnum;
                        packet_send_args->sessid = sessid;

                        /* quit upon receiving q command in the tty or
                         * when eof reached in file/sent in tty
                         */
                        if ((b->b[0] == 'q' && isatty(STDIN_FILENO)) ||
                            !(b->bwr)) {
                                packet_send_args->command = GOODBYE;
                                packet_send_args->data = NULL;
                                packet_send_args->ds = 0;

                                err = _client_packet_send(packet_send_args);
                                _check_err(err, "_client_packet_send: failed",
                                           _FATAL);
                                fsmstate = _FSM_CLOSING;
                        } else {
                                uint8_t *data = malloc(b->bwr);
                                memcpy(data, b->b, b->bwr);
                                packet_send_args->command = DATA;
                                packet_send_args->data = data;
                                packet_send_args->ds = b->bwr;
                                err = _client_packet_send(packet_send_args);
                                _check_err(err, "_client_packet_send: failed",
                                           _FATAL);
                                waitsack = !waitsack;
                        }

                        free(packet_send_args);
                }
        }

        return 0;
}
