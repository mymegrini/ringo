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
      struct sockaddr_in mdiff[NRING];
      int socktcp;
  } _entity;

  struct test_data {
    int count;
    int nring;
    char ring_check[NRING];
};


extern volatile struct test_data* test_data;

extern entity * ent;
extern _entity *_ent;
extern entity _ent_;

char *entitytostr(int ring);
void init_entity(char *id, uint16_t udp_listen, uint16_t tcp_listen, 
        char *mdiff_ip, uint16_t mdiff_port);
void create_ring();

void *insertion_server(void *args);
void *message_manager(void *args);
void* ring_tester(void *args);
void *mdiff_manager(void *args);


int join(const char *host, const char *tcpport);
int duplicate_rqst(const char *host, const char *tcpport);
void sendpacket_all(const char *content);
void sendpacket(const char *content, int ring);
void sendpacket_sockaddr(const char *content, const struct sockaddr_in *receiver);
void rm_ring(int ring);
int create_ring2(char *mdiff_ip, uint16_t mdiff_port);
int join2(const char *host, const char *tcpport);
int duplicate_rqst2(const char *host, const char *tcpport, const char *mdiff_ip, uint16_t mdiff_port);

void decrement_test_counter();


extern volatile const int * const ring_number;

#endif /* PROTOCOL_H */
