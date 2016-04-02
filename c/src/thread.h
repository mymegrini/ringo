#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>



struct threads {
    pthread_t tcp_server;
    pthread_t message_manager;
};

extern struct threads threads;


#endif /* THREAD_H */
