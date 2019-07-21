#include "_net.h"
#include <string.h>
#include <sys/select.h>
struct _client_state {
        _net_udp_conn_t *dialconn;
        uint32_t sessid;
        uint32_t seqnum;
        fd_set _rfds;
        fd_set rfds;
        _buf_t *b;
};

typedef struct _client_state _client_state_t;

int8_t _client_loop(_client_state_t *state);
