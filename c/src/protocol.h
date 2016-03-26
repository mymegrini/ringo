#ifndef PROTOCOL_H
#define PROTOCOL_H

typedef struct entity {
    char            id[9]; // 8th char max, 9th is 0
    char            ip_self[16];
    unsigned short  udp;
    unsigned short  tcp;
    char            ip_next[16];
    unsigned short  port_next;
    char            mdiff_ip[16];
    unsigned short  mdiff_port;
} entity;


void launch_insserv();
int insert(const char *host, const char *tcpport);
void sendpacket(char *content);

#endif /* PROTOCOL_H */
