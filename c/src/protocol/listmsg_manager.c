#include "listmsg_manager.h"

#include "../common.h"
#include "thread.h"

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include <pthread.h>


#ifndef LIST_MSG_MAXTIME
#define LIST_MSG_MAXTIME 5
#endif
#ifndef LIST_MSG_MANAGER_SLEEP
#define LIST_MSG_MANAGER_SLEEP 10
#endif
#ifndef LIST_MSG_MAXMSG
#define LIST_MSG_MAXMSG 5
#endif

static list             listmsg;
static time_t           t_begin;
static pthread_rwlock_t list_rwlock;


static void init_listmsg_manager() {
  pthread_rwlock_init(&list_rwlock, NULL);
  listmsg = new_list();
}



static int time_out(const char *idm, void *data)
{
  time_t *t_end = (time_t *)data;
  int spent = difftime(*t_end, t_begin);
  return spent >= LIST_MSG_MAXTIME;
}



void *listmsg_manager(void *nothing)
{
  init_listmsg_manager();
  while (1)
  {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    t_begin = t.tv_sec;
    sleep(LIST_MSG_MANAGER_SLEEP);
    int size = list_size(listmsg);
    verbose(REVERSE "%d idm in the list.\n" RESET, size);
    if (size > LIST_MSG_MAXMSG) {
      verbose(REVERSE "Cleaning idm list...\n" RESET);
      pthread_rwlock_wrlock(&list_rwlock);
      rm_all_if_free_data(listmsg, time_out, free);
      pthread_rwlock_unlock(&list_rwlock);
      int removed = size - list_size(listmsg);
      verbose(REVERSE "%d idm removed.\n" RESET, removed);
    }
  }
  return NULL;
}



int lookup(const char *idm)
{
  pthread_rwlock_rdlock(&list_rwlock);
  int res = mem(listmsg, idm);
  pthread_rwlock_unlock(&list_rwlock);
  if (res == 0) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    time_t *t = malloc(sizeof(time_t));
    *t = ts.tv_sec;
    pthread_rwlock_wrlock(&list_rwlock);
    insert(listmsg, idm, t);
    pthread_rwlock_unlock(&list_rwlock);
  }
  return res;
}

