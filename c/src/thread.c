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


void init_threads() {
    // lauch message managerthread
    verbose("Starting message manager...\n");
    pthread_create(&threads.message_manager, NULL, message_manager, NULL);
    verbose("Message manager started.\n");
    // lauch insertion server thread
    verbose("Starting insertion server...\n");
    pthread_create(&threads.tcp_server, NULL, insertion_server, NULL);
    verbose("Insertion manager started.\n");
    // launch ring tester thread
    verbose("Starting ring tester...\n");
    pthread_create(&threads.tcp_server, NULL, ring_tester, NULL);
    verbose("Ring tester started.\n");
}
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
