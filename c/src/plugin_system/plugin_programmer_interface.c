#include "plugin_programmer_interface.h"

/* #include "plugin_system.h" */

#ifdef JAVA_PLUGIN_SYSTEM
#include <jni.h>
#else
#include "../protocol/application.h"
#include "../protocol/protocol.h"
#endif



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


static void messageid(char* hash) {

    struct timeval time;
    uint64_t h = 5381;
    int i;
    uint8_t c;

    /* hash * 33 + c */
    // hashing time
    gettimeofday(&time, NULL);
    h = h * 33 + (uint16_t)time.tv_sec;
    h = h * 33 + (uint16_t)time.tv_usec;
    // hashing ip and port
    for(i=0; i<16; i++) h = h * 33 + ent->ip_self[i];  
    h = h * 33 + ent->udp;
    // hashing content
    // while((c = *content++)) h = h * 33 + c;

    // creating hash using alphanumerical characters
    for(i=0; i<8; i++){
        c = h % 62;
        if (c<10) hash[i] = c+48;      //digits
        else if (c<36) hash[i] = c-10+97; //lowercase letters
        else if (c<62) hash[i] = c-36+65; //uppercase letters
        else hash[i] = 0;
    
        h = h / 62;
    }
    hash[8] = 0;

    /*debug("messageid", "Created message id : %s.\n", hash);*/
}



static void makeappmessage(char* idm, char* buff, const char* idapp,
        const char* format, va_list aptr)
{
    char content[490];

    // creating message content
    vsnprintf(content, 490, format, aptr);
    
    // creating new id
    messageid(idm);

    // creating message
    if (strlen(content))
      snprintf(buff, 513, "APPL %s %s %s", idm, idapp, content);
    else snprintf(buff, 513, "APPL %s %s", idm, idapp);

    return;
}



void retransmit(const char *message)
{
  push_message(&plugin_manager, message);
}



void send_message(const char *idapp, const char *format, ...) 
{
  char idm[9], buff[513];
  va_list aptr;
  va_start(aptr, format);
  makeappmessage(idm, buff, idapp, format, aptr);
  va_end(aptr);
  push_message(&plugin_manager, buff);
}



#ifdef JAVA_PLUGIN_SYSTEM
static jobject get_ent(jobject psys)
{
  jclass psysClass = (*jvm.env)->GetObjectClass(jvm.env, jvm.obj);
  jfieldID fid = (*jvm.env)->GetFieldID(jvm.env, psysClass, "ent", "LEntity");
  if (fid == NULL)
    return NULL;
  return (*jvm.env)->GetObjectField(jvm.env, jvm.obj, fid);
}
#endif

int get_ring_number()
{
#ifdef java_plugin_system
#else
  return *ring_number;
#endif
}



char *get_id(char *id)
{
#ifdef JAVA_PLUGIN_SYSTEM
  jobject ent = get_ent(jvm.obj);
  if (!ent)
    return -12;
  jclass entclass = (*jvm.env)->getobjectclass(jvm.env, ent);
  jfieldid fid = (*jvm.env)->getfieldid(jvm.env, entclass, "id", "Ljava/lang/String;");
  if (fid == NULL)
    return NULL;
  jstring jid = (*jvm.env)->GetObjectField(jvm.env, ent, fid);
  const char *cid = (*jvm.env)->GetStringUTFChars(jvm.env, jid, NULL);
  strcpy(id, cid);
  (*jvm.env)->ReleaseStringUTFChars(jvm.env, jid, cid);
#else
  strcpy(id, ent->id);
#endif
  return id;
}



char *get_ip(char *ip)
{
#ifdef JAVA_PLUGIN_SYSTEM
  jobject ent = get_ent(jvm.obj);
  if (!ent)
    return -12;
  jclass entclass = (*jvm.env)->getobjectclass(jvm.env, ent);
  jfieldid fid = (*jvm.env)->getfieldid(jvm.env, entclass, "ip", "Ljava/lang/String;");
  if (fid == NULL)
    return NULL;
  jstring jip = (*jvm.env)->GetObjectField(jvm.env, ent, fid);
  const char *cip = (*jvm.env)->GetStringUTFChars(jvm.env, jid, NULL);
  strcpy(ip, cid);
  (*jvm.env)->ReleaseStringUTFChars(jvm.env, jid, cid);
#else
  strcpy(ip, ent->ip_self);
#endif
  return ip;
}



uint16_t get_udp()
{
  uint16_t udp;
#ifdef JAVA_PLUGIN_SYSTEM
  jobject ent = get_ent(jvm.obj);
  if (!ent)
    return -12;
  jclass entclass = (*jvm.env)->getobjectclass(jvm.env, ent);
  jfieldid fid = (*jvm.env)->getfieldid(jvm.env, entclass, "udp", "I");
  if (fid == NULL)
    return NULL;
#else
  udp = ent->udp;
#endif
  return udp;
}



struct xterm { 
  pid_t pid;
  int   pipe[2];
  FILE *in;
};



static void fifo_path(char *name)
{ strcpy(name, "/tmp/ringo_xterm");
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


