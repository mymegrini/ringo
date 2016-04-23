#ifndef PROTOCOL_INTERFACE_H
#define PROTOCOL_INTERFACE_H


extern void (*send_message)(char *, char *, ...);
extern void (*retransmit)(char *);

#endif /* PROTOCOL_INTERFACE_H */
