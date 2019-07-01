#define _POSIX_C_SOURCE (200809L)
#define _XOPEN_SOURCE (700)
#include <stdio.h>
#include <stdlib.h>

#include "_utils.h"

#define USAGE ("Usage: server port\nWhere port is < 65536")

int main(int argc, char **argv) {
        uintmax_t portnum;
        int8_t err;

        err = argc != 2;
        _check_err(err, USAGE, true);

        err = _parse_port(argv[1], &portnum);
        _check_err(err, "err, _parse_port: failed", true);

        err = portnum > 65535;
        _check_err(err, "err, port out of range", true);

        return 0;
}
