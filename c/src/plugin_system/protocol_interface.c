#include "protocol_interface.h"

#include "../protocol/application.h"
#include "../protocol/protocol.h"

void (*send_message)(const  char *, const char *, ...) = sendappmessage_all;
void (*retransmit)(const    char *)                    = sendpacket_all;

info_t * const info = (info_t * const)&_ent_;
/* info_t * const info = (info_t *)ent; */
