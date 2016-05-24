#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>

#include "protocol.h"



struct thread {
    pthread_t tcp_server;
    pthread_t message_manager;
    pthread_t ring_tester;
    pthread_t mdiff_manager;
    pthread_t shell;
    pthread_t listmsg_manager;
    pthread_t plugin_message_manager;
};

extern struct thread *thread;


void init_threads();
void close_threads();
void close_threads_and_shell();
void restart_mdiffmanager();
/* void restart_mdiffmanager_request(); */

void start_tcpserver();
void close_tcpserver();


struct test_mutex {
    pthread_mutex_t m;
    pthread_cond_t  c;
};

struct mutex {
    pthread_mutex_t   listmsg;
    pthread_rwlock_t  entity_rw;
    struct test_mutex test;
    pthread_mutex_t   mdiff;
    pthread_cond_t    mdiff_restart_cond;
};



extern struct mutex *mutex;


void init_mutex();
void wlock_entity();
void unlock_entity();
void rlock_entity();
void lock_mdiff();
void unlock_mdiff();

#endif /* THREAD_H */
