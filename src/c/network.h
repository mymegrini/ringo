#ifndef NETWORK_H
#define NETWORK_H

char *getIp(const char *hostname);

char *getLocalIp();

char *receptLine(const int sock);

#endif /* NETWORK_H */
