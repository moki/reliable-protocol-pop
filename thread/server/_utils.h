#pragma once

#include <inttypes.h>
#include <stdbool.h>

#define _FATAL (true)

extern int8_t _parse_port(char *portstr, uintmax_t *portdest);
extern void _check_err(int8_t errcode, char *msg, bool fatal);
