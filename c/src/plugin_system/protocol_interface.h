#ifndef PROTOCOL_INTERFACE_H
#define PROTOCOL_INTERFACE_H


#include <stdint.h>
#include "../protocol/protocol.h"




extern void (*send_message)(const char *, const char *, ...);
extern void (*retransmit)(const char *);

typedef struct info_t {
    const char     id[9];
    const char     ip_self[16];
    const uint16_t udp;
    const uint16_t tcp;
    const char     ip_next[NRING][16];
    const uint16_t port_next[NRING];
    const char     mdiff_ip[NRING][16];
    const uint16_t mdiff_port[NRING];
} info_t;

extern info_t * const info;

#endif /* PROTOCOL_INTERFACE_H */
