#include "_net.h"
#include "_utils.h"

#include <netdb.h>
#include <string.h>

int8_t _net_listen_udp(char *port, int *listener) {
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

        *listener = _listener;

        return 0;
}
