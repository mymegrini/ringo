#include "thread.h"

#include "protocol.h"
#include "common.h"

#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////
// LOCAL
////////////////////////////////////////////////////////////////////////////////

extern entity ent;
extern int nring;
extern _entity _ent;

////////////////////////////////////////////////////////////////////////////////
// GLOBAL
////////////////////////////////////////////////////////////////////////////////

struct threads threads;


void close_tcpserver() {
    verbose("Closing TCP server...\n");
    verbose("Closing TCP socket...\n");
    close(_ent.socktcp);
    verbose("TCP socket closed.\n");
    verbose("Closing TCP server thread...\n");
    pthread_cancel(threads.tcp_server);
    verbose("TCP server thread closed.\n");
    verbose("TCP server closed.\n");
}

void close_messagemanager() {
    pthread_cancel(threads.message_manager);
}
