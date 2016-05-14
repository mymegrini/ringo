#include "thread.h"

#include "common.h"

#include <unistd.h>
#include <pthread.h>
#include <signal.h>

////////////////////////////////////////////////////////////////////////////////
// LOCAL
////////////////////////////////////////////////////////////////////////////////

extern volatile int nring;
////////////////////////////////////////////////////////////////////////////////

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
static short volatile need_tcpserver = 1;
extern short volatile mdiff_working;

volatile int nmdiff_restart_request = 0;


void init_mutex() {
  verbose(REVERSE DIM "Initializing mutexes...\n" RESET);
  pthread_mutex_init(&mutex->listmsg, NULL);
  pthread_mutex_init(&mutex->test.m, NULL);
  pthread_cond_init(&mutex->test.c, NULL);
  pthread_rwlock_init(&mutex->entity_rw, NULL);
  pthread_mutex_init(&mutex->mdiff, NULL);
  pthread_cond_init(&mutex->mdiff_restart_cond, NULL);
  verbose(REVERSE "Mutexes initialized.\n" RESET);
}



static void close_messagemanager() {
  verbose(REVERSE "Closing message manager...\n" RESET);
  verbose(REVERSE "Closing message manager thread...\n" RESET);
  pthread_cancel(thread->message_manager);
  verbose(REVERSE "Message manager thread closed.\n" RESET);
  verbose(REVERSE "Closing UDP listening socket...\n" RESET);
  close(_ent->socklisten);
  _ent->socklisten = NEED_SOCKET;
  verbose(REVERSE "UDP listening socket closed.\n" RESET);
  verbose(REVERSE "Closing UDP sending socket...\n" RESET);
  close(_ent->socksend);
  _ent->socksend = NEED_SOCKET;
  verbose(REVERSE "UDP sending socket closed.\n" RESET);
  verbose(REVERSE "Message manager closed.\n" RESET);
}



/* static void close_tcpserver() { */
void close_tcpserver() {
  if (need_tcpserver) 
    return ;
  need_tcpserver = 1;
  verbose(REVERSE REVERSE "Closing TCP server...\n" RESET);
  verbose(REVERSE "Closing TCP server thread...\n" RESET);
  pthread_cancel(thread->tcp_server);
  verbose(REVERSE REVERSE "TCP server thread closed.\n" RESET);
  verbose(REVERSE "Closing TCP socket...\n" RESET);
  close(_ent->socktcp);
  _ent->socktcp = NEED_SOCKET;
  verbose(REVERSE "TCP socket closed.\n" RESET);
  verbose(REVERSE "TCP server closed.\n" RESET);
}



static void close_ringtester() {
  verbose(REVERSE "Closing ring tester...\n" RESET);
  /* pthread_mutex_lock(&mutex->test.m); */
  pthread_cancel(thread->ring_tester);
  /* pthread_mutex_unlock(&mutex->test.m); */
  verbose(REVERSE "Ring tester closed.\n" RESET);
}



static void close_mdiffmanager() {
  verbose(REVERSE "Closing multidiffusion message manager...\n" RESET);
  lock_mdiff();
  debug("close_mdiff_manager", "mdiff locked");
  pthread_cancel(thread->mdiff_manager);
  debug("close_mdiff_manager", "thread canceled");
  unlock_mdiff();
  debug("close_mdiff_manager", "mdiff unlocked");
  verbose(REVERSE "Multidiffusion message manager closed.\n" RESET);
}



/* static void start_tcpserver() { */
void start_tcpserver() {
  if (need_tcpserver) {
    need_tcpserver = 0;
    verbose(REVERSE "Starting insertion server...\n" RESET);
    pthread_create(&thread->tcp_server, NULL, insertion_server, NULL);
    verbose(REVERSE "Insertion server started.\n" RESET);
  }
}



static void start_messagemanager() {
    verbose(REVERSE "Starting message manager...\n" RESET);
    pthread_create(&thread->message_manager, NULL, message_manager, NULL);
    verbose(REVERSE "Message manager started.\n" RESET);
}



static void start_mdiffmanager() {
    verbose(REVERSE "Starting multidiffusion manager...\n" RESET);
    pthread_create(&thread->mdiff_manager, NULL, mdiff_manager, NULL);
    verbose(REVERSE "Multidiffusion message started.\n" RESET);
}



static void start_ringtester() {
    verbose(REVERSE "Starting ring tester...\n" RESET);
    pthread_create(&thread->ring_tester, NULL, ring_tester, NULL);
    verbose(REVERSE "Ring tester started.\n" RESET);
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



void lock_mdiff() {
  debug("lock_mdiff", "trying...");
  pthread_mutex_lock(&mutex->mdiff);
  debug("lock_mdiff", "mdiff locked");
}



void unlock_mdiff() {
  pthread_mutex_unlock(&mutex->mdiff);
}



void init_threads() {
  if (need_thread) {
    init_mutex();
    // lauch message managerthread
    start_messagemanager();
    // launch multidiffusion message manager
    start_mdiffmanager();
    // launch ring tester thread
    start_ringtester();
    // lauch insertion server thread
    start_tcpserver();

    need_thread = 0;
  }
  else if (need_tcpserver)
    start_tcpserver();
}


void close_threads()
{
  if (!need_thread) {
    verbose(REVERSE "Closing threads...\n" RESET);
    close_ringtester();
    close_messagemanager();
    close_mdiffmanager();
    close_tcpserver();
    need_thread = 1;
    verbose(REVERSE "Threads closed.\n" RESET);
  }
}



void close_threads_and_shell()
{
  close_threads();
  pthread_cancel(thread->shell);
}



static void *restart_mdiffmanager_request(void *args) {
  debug("restart_mdiffmanager_request", "requesting...");
  /* lock_mdiff(); */
  /* while (mdiff_working) */
  /*   pthread_cond_wait(&mutex->mdiff_restart_cond, &mutex->mdiff); */
  debug("restart_mdiffmanager_request", "killing mdiff_manager");
  /* pthread_kill(thread->mdiff_manager, SIGUSR1); */
  close_mdiffmanager();
  start_mdiffmanager();
  /* pthread_cancel(thread->mdiff_manager); */
  /* pthread_create(&thread->mdiff_manager, NULL, mdiff_manager, NULL); */
  debug("restart_mdiffmanager_request", "request sent.");
  nmdiff_restart_request = 0;
  /* unlock_mdiff(); */
  return NULL;
}



void restart_mdiffmanager() {
  if (++nmdiff_restart_request > 1) {
    debug("restart_mdiffmanager_request", "already requested, ignoring request.");
    return;
  }
  pthread_t t;
  pthread_create(&t, NULL, restart_mdiffmanager_request, NULL);
  /* lock_mdiff(); */
  /* pthread_cond_wait(&mutex->mdiff_restart_cond, &mutex->mdiff); */
  /* pthread_cancel(thread->mdiff_manager); */
  /* unlock_mdiff(); */
  /* if (nring > 0) */
}



