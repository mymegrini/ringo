#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>

#include "protocol.h"



struct thread {
    pthread_t tcp_server;
    pthread_t message_manager;
    pthread_t ring_tester;
    pthread_t mdiff_manager;
};

extern struct thread *thread;


void init_threads();
void close_threads();
void close_threads_and_exit();
void msg_exit();

struct test_mutex {
    pthread_mutex_t m;
    pthread_cond_t  c;
};

struct mutex {
    pthread_mutex_t   listmsg;
    pthread_rwlock_t  entity_rw;
    struct test_mutex test;
};



extern struct mutex *mutex;


void wlock_entity();
void unlock_entity();
void rlock_entity();

#endif /* THREAD_H */
