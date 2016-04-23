#include "protocol_interface.h"

#include "../protocol/application.h"
#include "../protocol/protocol.h"

void (*send_message)(char *, char *, ...) = sendappmessage_all;
void (*retransmit)(char *)                = sendpacket_all;
