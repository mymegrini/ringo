#ifndef NETWORK_H
#define NETWORK_H

#include <netinet/in.h>
char *getIp(const char *hostname);

char *getLocalIp();

char *receptLine(const int sock);

char *ipresize(const char *ip);
void ipresize_noalloc(char ipr[16], const char *ip);

/**
 * subscribe socket sock to multicast ip channel on port port.
 *
 * @param sock
 * @param port
 * @param ip
 * @return 0 if the binding failed, 1 else
 */
int multicast_subscribe(struct sockaddr_in *addr, int sock, int port, const char *ip);

/**
 * bind the socket sock for udp listening on port port.
 *
 * @param sock
 * @param port
 * @return 0 if the binding failed, 1 else
 */
int bind_udplisten(int sock, int port);

/**
 * Fill the struct sockaddr_in for communications with host on port port.
 *
 * @param addr, a ponter to the struct sockaddr_in
 * @param host an ip or hostname
 * @param is_ip if set to 0 host could be a hostname or an ip, else host is an
 * ip.
 * @return 1 if the struct sockaddr_in is filled, 0 else.
 */
int getsockaddr_in(struct sockaddr_in *addr, const char *host, int port, int is_ip);


#endif /* NETWORK_H */
