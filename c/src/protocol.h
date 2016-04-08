#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>



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

typedef struct _entity {
    int socklisten;
    int socksend;
    struct sockaddr_in receiver[NRING];
    int sockmdiff[NRING];
    int socktcp;
} _entity;

char *entitytostr(int ring);
void init_entity(char *id, uint16_t udp_listen, uint16_t tcp_listen, 
        char *mdiff_ip, uint16_t mdiff_port);
void create_ring();

void *insertion_server(void *args);
void *message_manager(void *args);
void* ring_tester(void *args);


int join(const char *host, const char *tcpport);
int dupplicate_rqst(const char *host, const char *tcpport);
void sendpacket_all(char *content);
void sendpacket(char *content, int ring);
void sendpacket_sockaddr(char *content, struct sockaddr_in *receiver);

extern volatile int nring;

#endif /* PROTOCOL_H */
