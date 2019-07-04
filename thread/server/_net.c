#include "_net.h"
#include "_utils.h"

#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

int8_t _net_listen_udp(char *port, int *sockfd) {
        struct addrinfo hints;
        struct addrinfo *server;
        struct addrinfo *cursor;
        int _listener;
        int8_t err;

        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_family = AF_UNSPEC;
        hints.ai_flags = AI_PASSIVE;

        err = getaddrinfo(NULL, port, &hints, &server);
        _check_err(err, "err, getaddrinfo: failed", _FATAL);

        for (cursor = server; cursor; cursor = cursor->ai_next) {
                _listener = socket(cursor->ai_family, cursor->ai_socktype,
                                   cursor->ai_protocol);
                _check_err(_listener, "err, socket: failed", !_FATAL);
                if (_listener == -1)
                        continue;

                err = bind(_listener, cursor->ai_addr, cursor->ai_addrlen);
                _check_err(err, "err, bind: failed", !_FATAL);
                if (err)
                        continue;
                break;
        }

        freeaddrinfo(server);

        if (!cursor)
                return -1;

        *sockfd = _listener;

        return 0;
}

int8_t _net_read_udp(int sockfd, uint8_t *b, size_t bs, ssize_t *read) {
        *read = recv(sockfd, b, bs, 0);
        if (*read == -1)
                return -1;
        _check_err(*read > _UDP_MAX_DATA_PAYLOAD ? -1 : 0,
                   "_net_read_udp: packet size", _FATAL);
        b[*read] = '\0';
        return 0;
}
