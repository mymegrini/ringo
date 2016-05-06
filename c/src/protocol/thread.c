#include "thread.h"

#include "common.h"

#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////
// LOCAL
////////////////////////////////////////////////////////////////////////////////

extern volatile int nring;

void close_tcpserver();
void close_messagemanager();
void close_ringtester();
void start_tcpserver();


////////////////////////////////////////////////////////////////////////////////
// GLOBAL
////////////////////////////////////////////////////////////////////////////////

struct thread _thread;
struct thread *thread = &_thread;
struct mutex _mutex;
struct mutex *mutex = &_mutex;

short volatile need_thread = 1;


static void init_mutex() {
  verbose("Initializing mutexes...\n");
  pthread_mutex_init(&mutex->listmsg, NULL);
  pthread_mutex_init(&mutex->test.m, NULL);
  pthread_cond_init(&mutex->test.c, NULL);
  pthread_rwlock_init(&mutex->entity_rw, NULL);
  verbose("Mutexes initialized.\n");
}



void init_threads() {
  if (need_thread) {
    init_mutex();
    // lauch message managerthread
    verbose("Starting message manager...\n");
    pthread_create(&thread->message_manager, NULL, message_manager, NULL);
    verbose("Message manager started.\n");
    // lauch insertion server thread
    start_tcpserver();
    // launch ring tester thread
    verbose("Starting ring tester...\n");
    pthread_create(&thread->ring_tester, NULL, ring_tester, NULL);
    verbose("Ring tester started.\n");
    need_thread = 0;
  }
}


void close_threads()
{
  verbose("Closing threads...\n");
  close_tcpserver();
  close_messagemanager();
  close_ringtester();
  need_thread = 1;
  verbose("Threads closed.\n");
}



void close_threads_and_exit()
{
  close_threads();
  printf("Bye bye!\n");
  exit(0);
}




void close_messagemanager() {
  verbose("Closing message manager...\n");
  verbose("Closing message manager thread...\n");
  pthread_cancel(thread->message_manager);
  verbose("Message manager thread closed.\n");
  verbose("Closing UDP listening socket...\n");
  close(_ent->socklisten);
  _ent->socklisten = NEED_SOCKET;
  verbose("UDP listening socket closed.\n");
  verbose("Closing UDP sending socket...\n");
  close(_ent->socksend);
  _ent->socksend = NEED_SOCKET;
  verbose("UDP sending socket closed.\n");
  verbose("Message manager closed.\n");
}





void close_tcpserver() {
  verbose("Closing TCP server...\n");
  verbose("Closing TCP server thread...\n");
  pthread_cancel(thread->tcp_server);
  verbose("TCP server thread closed.\n");
  verbose("Closing TCP socket...\n");
  close(_ent->socktcp);
  _ent->socktcp = NEED_SOCKET;
  verbose("TCP socket closed.\n");
  verbose("TCP server closed.\n");
}


void close_ringtester() {
  verbose("Closing ring tester...\n");
  pthread_cancel(thread->ring_tester);
  verbose("Ring tester closed.\n");
}



void start_tcpserver() {
  verbose("Starting insertion server...\n");
  pthread_create(&thread->tcp_server, NULL, insertion_server, NULL);
  verbose("Insertion manager started.\n");
}

void rlock_entity() {
  pthread_rwlock_rdlock(&mutex->entity_rw);
}

void unlock_entity() {
  pthread_rwlock_unlock(&mutex->entity_rw);
}

void wlock_entity() {
  pthread_rwlock_wrlock(&mutex->entity_rw);
}
