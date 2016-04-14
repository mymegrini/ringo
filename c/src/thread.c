#include "thread.h"

#include "common.h"

#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////
// LOCAL
////////////////////////////////////////////////////////////////////////////////

extern entity ent;
extern volatile int nring;
extern _entity _ent;

////////////////////////////////////////////////////////////////////////////////
// GLOBAL
////////////////////////////////////////////////////////////////////////////////

struct threads threads;
struct mutexes _mutexes;
struct mutexes *mutex = &_mutexes;


void init_mutexes() {
    verbose("Initializing mutexes...\n");
    pthread_mutex_init(&mutex->listmsg, NULL);
    pthread_mutex_init(&mutex->nring, NULL);
    for (int i = 0; i < NRING; ++i)
        pthread_mutex_init(&mutex->receiver[i], NULL);
    pthread_mutex_init(&mutex->test.m, NULL);
    pthread_cond_init(&mutex->test.c, NULL);
    verbose("Mutexes initialized.\n");
}



void init_threads() {
    init_mutexes();
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
    pthread_create(&threads.ring_tester, NULL, ring_tester, NULL);
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


void actualize_nring(int n) {
    pthread_mutex_lock(&mutex->nring);
    nring = n;
    pthread_mutex_unlock(&mutex->nring);
    debug("actualize_nring", "nring = %d", nring+1);
}


int getnring() {
    pthread_mutex_lock(&mutex->nring);
    int n = nring;
    pthread_mutex_unlock(&mutex->nring);
    return n;
}

void actualize_receiver(int ring, struct sockaddr_in *receiver) {
    pthread_mutex_lock(&mutex->receiver[ring]);
    _ent.receiver[ring] = *receiver;
    pthread_mutex_unlock(&mutex->receiver[ring]);

}

struct sockaddr_in *getreceiver(int ring) {
    struct sockaddr_in *receiver;
    pthread_mutex_lock(&mutex->receiver[ring]);
    receiver = &_ent.receiver[ring];
    pthread_mutex_unlock(&mutex->receiver[ring]);
    return receiver;
}
