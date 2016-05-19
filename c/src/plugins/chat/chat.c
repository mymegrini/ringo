#include "../../common.h"
#include "../../plugin_system/plugin_programmer_interface.h"

#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <ctype.h>

#include <pthread.h>



////////////////////////////////////////////////////////////////////////////////
// GLOBALS
////////////////////////////////////////////////////////////////////////////////
static void help(char *argv0);
static void usage(char *argv0);
static void send_chat(const char *mess);
static void print_chat(const char *name, const char *content);
static void unpadstrn(char *unpadded, const char *str, int len);
static void unpadstr(char *unpadded, const char *str);
static void padstr(char *padded, const char *str, int size);
static void outputto(int fd);
static int  output_term();
static void close_outputterm();
static void* terminal_chat(void *arg);





#define   MSIZE        485
#define   IDAPP_CHAT   "CHAT####"


static int   chat_fd = STDOUT_FILENO;

//// END OF GLOBALS




////////////////////////////////////////////////////////////////////////////////
// PLUGIN DATAS
////////////////////////////////////////////////////////////////////////////////

#define CHAT_TYPE "CHAT####"

static int cmd_chat(int argc, char **argv);
static int action_chat(const char *message, const char *content, int lookup_flag);
static void close_plugin();

char id[9];
char padded_id[9];



PluginCommand_t pcmd_chat = {
  "chat",
  "Chat plugin.",
  cmd_chat
};



PluginAction_t paction_chat = {
  CHAT_TYPE,
  "Plugins chat message.",
  &action_chat
};



Plugin plug_chat = {
  1,
  &pcmd_chat,
  1,
  &paction_chat,
  close_plugin
};



int init_chat(PluginManager *p)
{
  get_id(id);
  padstr(padded_id, id, 8);
  return plugin_register(p, "chat", &plug_chat);
}

static void close_plugin()
{
  if (chat_fd != STDOUT_FILENO)
    close_outputterm();
}



int action_chat(const char *mess, const char *content, int lookup_flag) {

  if (lookup_flag)
    return 1;

  int valid_size = 1;
  for (int i = 0; i < 3; ++i)
    valid_size &= isdigit(content[i]);

  if (content[3] != ' ' || !valid_size ||
      content[12] != ' '){
    debug(RED "action_chat", RED "message content \"%s\" not valid for application.",
        content);
    return 1;
  }
  char name[9];
  unpadstrn(name, &content[4], 8);
  int size = atoi(content);
  if (size > MSIZE || size < 0) {
    debug("print_chat", "Size error in message content (%d > MAXSIZE = %d || size < 0):"
        "\n%s\n", size, MSIZE, content);
    return 1;
  }
  char chat_mess[MSIZE+1];
  strncpy(chat_mess, &content[13], size);
  chat_mess[size] = 0;
  print_chat(name, chat_mess);
  retransmit(mess);
  return 0;
}



#define   OPT_HELP       'h'
#define   OPTL_HELP      "help"
#define   OPT_MESS       'm'
#define   OPTL_MESS      "message"
#define   OPT_TERMOUT    't'
#define   OPTL_TERMOUT   "terminal"
#define   OPT_STDOUT     's'
#define   OPTL_STDOUT    "stdin"

#define   OPT_STRING     "hm:ts"


static struct option longopts[] = {
  {OPTL_HELP,    no_argument,       0, OPT_HELP},
  {OPTL_MESS,    required_argument, 0, OPT_MESS},
  {OPTL_STDOUT,  no_argument,       0, OPT_STDOUT},
  {OPTL_TERMOUT, no_argument,       0, OPT_TERMOUT},
  {0,            0,                 0, 0}
};


#define FLAG_MESS 1
#define FLAG_STDOUT  2
#define FLAG_TERMOUT  4

int cmd_chat(int argc, char **argv)
{
  char message[481] = "";
  if (argc == 1) {
    return 0;
  }
  int c, indexptr;
  short flag = 0;
  optind = 0;
  int optionmess = 0;
  while ((c = getopt_long(argc, argv, OPT_STRING,
          longopts, &indexptr)) != -1) {
    switch (c) {
      case OPT_HELP:
        help(argv[0]);
        return 0;
        break;
      case OPT_MESS:
        flag |= FLAG_MESS;
        strncpy(message, optarg, 481);
        optionmess = 1;
        break;
      case OPT_STDOUT:
        flag &= ~FLAG_TERMOUT;
        flag |= FLAG_STDOUT;
        break;
      case OPT_TERMOUT:
        flag &= ~FLAG_STDOUT;
        flag |= FLAG_TERMOUT;
        break;
      default:
        usage(argv[0]);
        return 1;
        break;
    }
  }

  if (!optionmess && optind != argc && optind != -1) {
    debug("cmd_chat", "optind: %d, argc: %d", optind, argc);
    flag |= FLAG_MESS;
    int len = 0;
    for (int i = optind; i < argc - 1; ++i) {
      int slen = 1 + strlen(argv[optind]);
      if (len + slen >  MSIZE)
        break;
      len += slen;
      strcat(message, argv[i]);
      message[len-1] = ' ';
    }
    int slen = strlen(argv[argc-1]);
    if (len + slen <= MSIZE)
      strcat(message, argv[argc-1]);
  }

  if (flag & FLAG_STDOUT)
    outputto(STDOUT_FILENO);
  else if (flag & FLAG_TERMOUT)
    output_term();
  if (flag & FLAG_MESS) {
    send_chat(message);
  }
  return 0;
}

//// END OF PLUGIN DATAS


////////////////////////////////////////////////////////////////////////////////
// TOOLS
////////////////////////////////////////////////////////////////////////////////

static void print_chat(const char *name, const char *content)
{
  char uname[9];
  unpadstr(uname, name);
  dprintf(chat_fd, UNDERLINED "%-8s:" RESET " %s\n", name, content);
}





static void send_chat(const char *mess)
{
  unsigned len = strlen(mess);
  unsigned size = MSIZE < len ? MSIZE : len;
  char ssize[4] = {0};
  for (int i = 0; i < 3; ++i) {
    ssize[2-i] = '0' + size % 10;
    size /= 10;
  }
  send_message(IDAPP_CHAT, "%s %s %s", ssize, padded_id, mess);
  print_chat("You", mess);
}



static void usage(char *argv0)
{
  printf("Usage:\t%s [-m message] [-h]\n", argv0);
}



static void help(char *argv0)
{
  usage(argv0);
}



static void padstr(char *padded, const char *str, int size)
{
  int len = strlen(str);
  int pad = size - len;
  int i;
  for (i = 0; i < pad; ++i)
    padded[i] = ' ';
  strncpy(&padded[i], str, size < len ? size : len);
  padded[size] = 0;
}



static void unpadstr(char *unpadded, const char *str)
{
  while (*str == ' ')
    ++str;
  strcpy(unpadded, str);
}



static void unpadstrn(char *unpadded, const char *str, int len)
{
  while (*str == ' ' && --len >= 0)
    ++str;
  strncpy(unpadded, str, len);
  unpadded[len] = 0;
}


static void outputto(int fd)
{
  if (chat_fd != STDOUT_FILENO)
    close_outputterm();
  chat_fd = fd;
}



static xterm     x;
static pthread_t terminal_communication;



static int output_term()
{
  if (init_xterm_communication(&x) == -1)
    return 0;
  else {
    outputto(xterm_getoutput(x));
    pthread_create(&terminal_communication, NULL, terminal_chat, x);
  }
  return 1;
}


static void* terminal_chat(void *arg)
{
  xterm x = arg;
  while (1) {
      size_t n = 0;
      char *line;
      ssize_t r = xterm_getline(x, &line, &n);
      line[r-1] = 0;
      send_chat(line);
      free(line);
  }
  return NULL;
}



static void close_outputterm()
{
  if (chat_fd != STDOUT_FILENO) {
    pthread_cancel(terminal_communication);
    xterm_close(&x);
  }
}
//// END OF TOOLS


