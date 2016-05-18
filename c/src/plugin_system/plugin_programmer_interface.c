#include "plugin_programmer_interface.h"

/* #include "plugin_system.h" */


#include "../protocol/application.h"
#include "../protocol/protocol.h"



#ifdef DEBUG
#include "../protocol/listmsg_manager.h"
#include "../common.h"
#endif


#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>



extern void push_message(PluginManager *plug_manager, const char *message);

void send_message(const char *idapp, const char *format, ...) 
{
  char idm[9], buff[513];
  va_list aptr;
  va_start(aptr, format);
  makeappmessage(idm, buff, idapp, format, aptr);
  va_end(aptr);
#ifdef DEBUG
    int r =
#endif
      lookup(idm);
#ifdef DEBUG
    if (r==1) debug("makemessage", "Detected a hash collision: %s\n", idm);
#endif
  push_message(&plugin_manager, buff);
}



info_t * const info = (info_t * const)&_ent_;


struct xterm { 
  pid_t pid;
  int   pipe[2];
  FILE *in;
};



static void fifo_path(char *name)
{
    strcpy(name, "/tmp/ringo_xterm");
    struct timeval t;
    gettimeofday(&t, NULL);
    char istr[20];
    int n = t.tv_sec, i = 0;
    while (i < 20 && n > 0) {
      istr[i++] = '0' + n % 10;
      n /= 10;
    }
    istr[i] = 0;
    strcat(name, istr);
    i = 0; n = t.tv_usec;
    while (i < 20 && n > 0) {
      istr[i++] = '0' + n % 10;
      n /= 10;
    }
    istr[i] = 0;
    strcat(name, istr);
}



int init_xterm_communication(xterm *x)
{
    char path[60], pathin[60];
    fifo_path(path);
    mkfifo(path, 0600);
    fifo_path(pathin);
    mkfifo(pathin, 0600);
    char cat_cmd[180];
    strcpy(cat_cmd, "/bin/cat ");
    strcat(cat_cmd, path);
    strcat(cat_cmd, "& tee ");
    strcat(cat_cmd, pathin);
    strcat(cat_cmd, " > /dev/null");
    int fd, fdin;
    sigset_t mask;
    pid_t p = fork();
    switch (p) {
        case -1:
            fprintf(stderr, "Fork error.\n");
            break;
        case 0:
            sigfillset(&mask);
            sigprocmask(SIG_SETMASK, &mask, NULL);
            execlp("xterm", "xterm", "-e", cat_cmd, NULL);
            printf("FAILURE\n");
            break; 
        default:
            if ((fd = open(path, O_WRONLY)) == -1) {
                fprintf(stderr, "Can't open pipe\n");
                return -1;
            }
            if ((fdin = open(pathin, O_RDONLY)) == -1) {
                fprintf(stderr, "Can't open pipe\n");
                return -1;
            }
            xterm s = malloc(sizeof(struct xterm));
            s->pid     = p;
            s->pipe[0] = fdin;
            s->pipe[1] = fd;
            s->in = fdopen(fdin, "r");
            *x = s;
            return 0;
            break;
    }
    return -1;
}


int xterm_printf(xterm x, char *format, ...)
{
  va_list aptr;
  va_start(aptr, format);
  int r = vdprintf(x->pipe[1], format, aptr);
  va_end(aptr);
  return r;
}



int xterm_read(xterm x, char *buff, size_t count)
{
  return read(x->pipe[0], buff, count);
}



ssize_t xterm_getline(xterm x, char **line, size_t *n)
{
      return getline(line, n, x->in);
}



int xterm_getoutput(xterm x)
{
  return x->pipe[1];
}



pid_t xterm_getpid(xterm x)
{
  return x->pid;
}



void xterm_close(xterm *x)
{
  xterm s = *x;
  fclose(s->in);
  close(s->pipe[0]);
  close(s->pipe[1]);
  kill(s->pid, SIGKILL);
  free(s);
}


