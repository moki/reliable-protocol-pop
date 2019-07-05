#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "_utils.h"

int8_t _parse_port(char *portstr, uintmax_t *portdest) {
        if (!portstr)
                return -1;
        if (!portdest)
                return -1;

        uintmax_t portnum = strtoumax(portstr, NULL, 10);
        if (portnum == UINTMAX_MAX && errno == ERANGE) {
                return -1;
        }

        *portdest = portnum;
        return 0;
}

void _check_err(int64_t errcode, char *msg, bool fatal) {
        if (errcode < 0)
                fprintf(stderr, "%s\n", msg);
        if (errcode < 0 && fatal)
                exit(EXIT_FAILURE);
}
