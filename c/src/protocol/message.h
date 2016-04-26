#ifndef MESSAGE_H
#define MESSAGE_H

#include <netinet/in.h>

int parsemsg(char *message);
void sendmessage_all(const char *type, const char *format, ...);
void sendmessage(int ring, const char *type, const char *format, ...);
void sendmessage_sockaddr(const struct sockaddr_in *receiver, const char *type, const char *format, ...);


#endif /* MESSAGE_H */
