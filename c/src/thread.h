#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>



struct threads {
    pthread_t tcp_server;
    pthread_t message_manager;
    pthread_t ring_tester;
};

extern struct threads threads;


void close_tcpserver();
void close_messagemanager();
void init_threads();


#endif /* THREAD_H */
