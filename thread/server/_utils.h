#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <unistd.h>

#define _FATAL (true)

struct _buf {
        ssize_t bwr;
        uint8_t *b;
        size_t bs;
};

typedef struct _buf _buf_t;

extern int8_t _parse_port(char *portstr, uintmax_t *portdest);
extern void _check_err(int64_t errcode, char *msg, bool fatal);
