#ifndef MESSAGE_H
#define MESSAGE_H

#include <netinet/in.h>

int parsemsg(char *message);
void sendmessage_all(char *type, char *format, ...);
void sendmessage(int ring, char *type, char *format, ...);
void sendmessage_sockaddr(struct sockaddr_in *receiver, char *type, char *format, ...);


#endif /* MESSAGE_H */
