#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>



struct threads {
    pthread_t tcp_server;
    pthread_t message_manager;
};

extern struct threads threads;


void close_tcpserver();
void close_messagemanager();


#endif /* THREAD_H */
