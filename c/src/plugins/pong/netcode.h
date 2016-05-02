#ifndef NETCODE_H
#define NETCODE_H

int parsePong(const char *message, const char *content, int lookup_flag);
void loginPong();
void sendUpdate();

#endif
