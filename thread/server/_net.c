#include "_net.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int8_t _net_udp_listen(_net_udp_conn_t *conn) {
        if (!conn)
                return -1;
        struct addrinfo hints;
        struct addrinfo *server;
        struct addrinfo *cursor;
        size_t portstrl;
        char *portstr;
        int sk;
        int8_t err;

        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_family = AF_UNSPEC;
        hints.ai_flags = AI_PASSIVE;

        portstrl = snprintf(NULL, 0, "%u", conn->destport);
        portstr = malloc(portstrl + 1);
        snprintf(portstr, portstrl + 1, "%u", conn->destport);

        err = getaddrinfo(NULL, portstr, &hints, &server);
        _check_err(err != 0 ? -1 : 0, "err, getaddrinfo: failed", _FATAL);

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

        free(portstr);
        freeaddrinfo(server);

        if (!cursor)
                return -1;

        conn->sk = sk;

        return 0;
}

int8_t _net_udp_dial(_net_udp_conn_t *conn) {
        if (!conn)
                return -1;

        struct addrinfo hints;
        struct addrinfo *client;
        struct addrinfo *cursor;
        size_t portstrl;
        char *portstr;
        int8_t err;
        int reuseaddr;
        int sk;

        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_family = AF_UNSPEC;
        hints.ai_flags = AI_PASSIVE;

        portstrl = snprintf(NULL, 0, "%u", conn->destport);
        portstr = malloc(portstrl + 1);
        snprintf(portstr, portstrl + 1, "%u", conn->destport);

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;

        err = getaddrinfo(conn->destaddrstr, portstr, &hints, &client);
        _check_err(err != 0 ? -1 : 0, "getaddrinfo: failed", _FATAL);

        reuseaddr = 1;
        for (cursor = client; cursor; cursor = cursor->ai_next) {
                sk = socket(cursor->ai_family, cursor->ai_socktype,
                            cursor->ai_protocol);
                _check_err(sk, "socket: failed", !_FATAL);
                if (sk == -1)
                        continue;
                err = setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, &reuseaddr,
                                 sizeof(int));
                _check_err(err, "setsockopt: failed", !_FATAL);
                if (err)
                        continue;
                err = connect(sk, cursor->ai_addr, cursor->ai_addrlen);
                _check_err(err, "connect: failed", !_FATAL);
                if (err)
                        continue;
                break;
        }

        free(portstr);
        freeaddrinfo(client);

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

int8_t _net_udp_write(_net_udp_conn_t *conn, _buf_t *b) {
        if (!conn)
                return -1;
        if (!b)
                return -1;
        b->bwr = send(conn->sk, b->b, b->bs, 0);
        if (b->bwr < 0) {
                perror("write failed: ");
                return -1;
        }

        return 0;
}

int8_t _net_udp_conn_setsrcaddrstr(_net_udp_conn_t *conn) {
        if (!conn)
                return -1;
        struct sockaddr *address = (struct sockaddr *)&(conn->addr);
        void *_address;
        if (address->sa_family == AF_INET)
                _address = &(((struct sockaddr_in *)address)->sin_addr);
        else
                _address = &(((struct sockaddr_in6 *)address)->sin6_addr);
        void *ptr = (char *)inet_ntop(conn->addr.ss_family, _address,
                                      conn->srcaddrstr, INET6_ADDRSTRLEN);
        if (!ptr)
                return -1;
        return 0;
}

int8_t _net_udp_conn_setsrcport(_net_udp_conn_t *conn) {
        if (!conn)
                return -1;
        struct sockaddr *address = (struct sockaddr *)&(conn->addr);
        if (address->sa_family == AF_INET) {
                conn->srcport =
                        ntohs(((struct sockaddr_in *)address)->sin_port);
        } else {
                conn->srcport =
                        ntohs(((struct sockaddr_in6 *)address)->sin6_port);
        }
        return 0;
}

int8_t _net_udp_conn_setdestport(_net_udp_conn_t *conn, uint16_t port) {
        if (!conn)
                return -1;
        conn->destport = port;
        return 0;
}

int8_t _net_udp_conn_setdestaddrstr(_net_udp_conn_t *conn,
                                    char addrstr[INET6_ADDRSTRLEN]) {
        if (!conn)
                return -1;
        if (!addrstr)
                return -1;

        if (!strncpy(conn->destaddrstr, addrstr, INET6_ADDRSTRLEN))
                return -1;

        return 0;
}
