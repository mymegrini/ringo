#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define NRING 2

typedef struct entity {
    char            id[9]; // 8th char max, 9th is 0
    char            ip_self[16];
    uint16_t        udp;
    uint16_t        tcp;
    char            ip_next[NRING][16];
    uint16_t        port_next[NRING];
    char            mdiff_ip[NRING][16];
    uint16_t        mdiff_port[NRING];
} entity;


void launch_insserv();
int insert(const char *host, const char *tcpport);
void sendpacket(char *content);

extern int nring;

#endif /* PROTOCOL_H */
