#include "thread.h"

#include "common.h"

#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////
// LOCAL
////////////////////////////////////////////////////////////////////////////////

extern volatile int nring;

/* void close_tcpserver(); */
/* void close_messagemanager(); */
/* void close_ringtester(); */
/* void start_tcpserver(); */


////////////////////////////////////////////////////////////////////////////////
// GLOBAL
////////////////////////////////////////////////////////////////////////////////

struct thread _thread;
struct thread *thread = &_thread;
struct mutex _mutex;
struct mutex *mutex = &_mutex;

short volatile need_thread = 1;


static void init_mutex() {
  verbose(UNDERLINED "Initializing mutexes...\n" RESET);
  pthread_mutex_init(&mutex->listmsg, NULL);
  pthread_mutex_init(&mutex->test.m, NULL);
  pthread_cond_init(&mutex->test.c, NULL);
  pthread_rwlock_init(&mutex->entity_rw, NULL);
  verbose(UNDERLINED "Mutexes initialized.\n" RESET);
}



static void close_messagemanager() {
  verbose(UNDERLINED "Closing message manager...\n" RESET);
  verbose(UNDERLINED "Closing message manager thread...\n" RESET);
  pthread_cancel(thread->message_manager);
  verbose(UNDERLINED "Message manager thread closed.\n" RESET);
  verbose(UNDERLINED "Closing UDP listening socket...\n" RESET);
  close(_ent->socklisten);
  _ent->socklisten = NEED_SOCKET;
  verbose(UNDERLINED "UDP listening socket closed.\n" RESET);
  verbose(UNDERLINED "Closing UDP sending socket...\n" RESET);
  close(_ent->socksend);
  _ent->socksend = NEED_SOCKET;
  verbose(UNDERLINED "UDP sending socket closed.\n" RESET);
  verbose(UNDERLINED "Message manager closed.\n" RESET);
}





static void close_tcpserver() {
  verbose(UNDERLINED "Closing TCP server...\n" RESET);
  verbose(UNDERLINED "Closing TCP server thread...\n" RESET);
  pthread_cancel(thread->tcp_server);
  verbose(UNDERLINED "TCP server thread closed.\n" RESET);
  verbose(UNDERLINED "Closing TCP socket...\n" RESET);
  close(_ent->socktcp);
  _ent->socktcp = NEED_SOCKET;
  verbose(UNDERLINED "TCP socket closed.\n" RESET);
  verbose(UNDERLINED "TCP server closed.\n" RESET);
}


static void close_ringtester() {
  verbose(UNDERLINED "Closing ring tester...\n" RESET);
  pthread_cancel(thread->ring_tester);
  verbose(UNDERLINED "Ring tester closed.\n" RESET);
}



static void close_mdiffmanager() {
  verbose(UNDERLINED "Closing multidiffusion message manager...\n" RESET);
  pthread_cancel(thread->mdiff_manager);
  verbose(UNDERLINED "Multidiffusion message manager closed.\n" RESET);
}


static void start_tcpserver() {
  verbose(UNDERLINED "Starting insertion server...\n" RESET);
  pthread_create(&thread->tcp_server, NULL, insertion_server, NULL);
  verbose(UNDERLINED "Insertion manager started.\n" RESET);
}

static void start_messagemanager() {
    verbose(UNDERLINED "Starting message manager...\n" RESET);
    pthread_create(&thread->message_manager, NULL, message_manager, NULL);
    verbose(UNDERLINED "Message manager started.\n" RESET);
}



static void start_ringtester() {
    verbose(UNDERLINED "Starting ring tester...\n" RESET);
    pthread_create(&thread->ring_tester, NULL, ring_tester, NULL);
    verbose(UNDERLINED "Ring tester started.\n" RESET);
}


static void start_mdiffmanager() {
    verbose(UNDERLINED "Starting multidiffusion message manager...\n" RESET);
    pthread_create(&thread->mdiff_manager, NULL, mdiff_manager, NULL);
    verbose(UNDERLINED "Multidiffusion message manager started.\n" RESET);
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



void init_threads() {
  if (need_thread) {
    init_mutex();
    // lauch message managerthread
    start_messagemanager();
    // lauch insertion server thread
    start_tcpserver();
    // launch ring tester thread
    start_ringtester();
    // launch multidiffusion message manager
    start_mdiffmanager();

    need_thread = 0;
  }
}


void close_threads()
{
  if (!need_thread) {
    verbose(UNDERLINED "Closing threads...\n" RESET);
    close_tcpserver();
    close_messagemanager();
    close_mdiffmanager();
    close_ringtester();
    need_thread = 1;
    verbose(UNDERLINED "Threads closed.\n" RESET);
  }
}



void close_threads_and_shell()
{
  close_threads();
  pthread_cancel(thread->shell);
}

