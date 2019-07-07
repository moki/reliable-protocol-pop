#include "_net.h"

#include <arpa/inet.h>
#include <string.h>

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

int8_t _net_udp_conn_getaddr(_net_udp_conn_t *conn) {
        if (!conn)
                return -1;
        struct sockaddr *address = (struct sockaddr *)&(conn->addr);
        void *_address;
        if (address->sa_family == AF_INET)
                _address = &(((struct sockaddr_in *)address)->sin_addr);
        else
                _address = &(((struct sockaddr_in6 *)address)->sin6_addr);
        void *ptr = (char *)inet_ntop(conn->addr.ss_family, _address,
                                      conn->addrstr, INET6_ADDRSTRLEN);
        if (!ptr)
                return -1;
        return 0;
}
