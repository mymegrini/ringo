#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>

#include "protocol.h"



struct threads {
    pthread_t tcp_server;
    pthread_t message_manager;
    pthread_t ring_tester;
};

extern struct threads threads;


void close_tcpserver();
void close_messagemanager();
void init_threads();

struct test_mutex {
    pthread_mutex_t m;
    pthread_cond_t c;
};

struct mutexes {
    pthread_mutex_t listmsg;
    pthread_mutex_t receiver[NRING];
    pthread_mutex_t nring;
    struct test_mutex test;
};



extern struct mutexes *mutex;


void actualize_nring(int n);
void actualize_receiver(int ring, struct sockaddr_in *receiver);
int getnring();

#endif /* THREAD_H */
