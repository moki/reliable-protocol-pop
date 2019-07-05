#include "_net.h"

#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

int8_t _net_udp_listen(_net_udp_conn_t *conn) {
        if (!conn)
                return -1;
        struct addrinfo hints;
        struct addrinfo *server;
        struct addrinfo *cursor;
        int sk;
        int8_t err;

        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_family = AF_UNSPEC;
        hints.ai_flags = AI_PASSIVE;

        err = getaddrinfo(NULL, conn->port, &hints, &server);
        _check_err(err, "err, getaddrinfo: failed", _FATAL);

        for (cursor = server; cursor; cursor = cursor->ai_next) {
                sk = socket(cursor->ai_family, cursor->ai_socktype,
                            cursor->ai_protocol);
                _check_err(sk, "err, socket: failed", !_FATAL);
                if (sk == -1)
                        continue;

                err = bind(sk, cursor->ai_addr, cursor->ai_addrlen);
                _check_err(err, "err, bind: failed", !_FATAL);
                if (err)
                        continue;
                break;
        }

        freeaddrinfo(server);

        if (!cursor)
                return -1;

        conn->sk = sk;

        return 0;
}

int8_t _net_udp_read(_net_udp_conn_t *conn, _buf_t *b) {
        if (!conn)
                return -1;
        if (!b)
                return -1;
        b->bwr = recvfrom(conn->sk, b->b, b->bs, 0,
                          (struct sockaddr *)&(conn->addr), &(conn->addr_len));
        if (b->bwr < 0)
                return -1;
        return 0;
}
