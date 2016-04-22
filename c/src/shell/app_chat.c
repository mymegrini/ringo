#include "../protocol/protocol.h"
#include "../protocol/message.h"
#include "../protocol/common.h"
#include "../protocol/network.h"
#include "../protocol/thread.h"
#include "../protocol/application.h"
#include "shell.h"

#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>


// chat message: "APPL idm CHAT#### SIZE NAME CONTENT"

#define   MSIZE        485
#define   IDAPP_CHAT   "CHAT####"


static int chat_fd = STDOUT_FILENO;








static void print_chat(char *name, char *content) {
  dprintf(chat_fd, UNDERLINED "%s:" RESET " %s\n", name, content);
}



void action_chat(char *mess, char *content, int lookup_flag) {

  if (lookup_flag)
    return;

  if (content[3] != ' ' || !isnumericn(content, 3) ||
      content[12] != ' '){
    debug(RED "action_chat", RED "message \"%s\" not valid for application.",
        mess);
    return;
  }
  char name[9];
  unpadstrn(name, &content[4], 8);
  int size = atoi(content);
  if (size > MSIZE || size < 0) {
    debug("print_chat", "Size error in message content (%d > MAXSIZE = %d || size < 0):"
        "\n%s\n", size, MSIZE, content);
    return;
  }
  char chat_mess[MSIZE+1];
  strncpy(chat_mess, &content[13], size);
  chat_mess[size] = 0;
  print_chat(name, chat_mess);
  sendpacket_all(mess);
}



static void send_chat(char *mess) {
  unsigned len = strlen(mess);
  unsigned size = MSIZE < len ? MSIZE : len;
  char ssize[4];
  itoa(ssize, 4, size);
  char name[9];
  padstr(name, ent->id, 8);
  sendappmessage_all(IDAPP_CHAT, "%s %s %s", ssize, name, mess);
  print_chat(name, mess);
}



#define   OPT_HELP    'h'
#define   OPTL_HELP   "help"
#define   OPT_MESS    'm'
#define   OPTL_MESS   "message"

#define   OPT_STRING  "hm:"

static struct option longopts[] = {
  {OPTL_HELP,     no_argument, 0, OPT_HELP},
  {OPTL_MESS,     required_argument, 0, OPT_MESS},
  {0, 0, 0, 0}
};



static void usage(char *argv0)
{
  printf("Usage:\t%s [-m message] [-h]\n", argv0);
}



static void help(char *argv0)
{
  usage(argv0);
}



static void getmessage(char *message)
{
  char *line = readline(UNDERLINED "Enter your message:\n");
  strncpy(message, line, 481);
  free(line);
}


#define FLAG_MESS 1

int cmd_chat(int argc, char **argv)
{
  int c, indexptr;
  short flag = 0;
  char message[481];
  optind = 0;
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
        break;
      default:
        usage(argv[0]);
        return 1;
        break;
    }
  }

  if (flag & FLAG_MESS) {
    send_chat(message);
  }
  else {
    getmessage(message);
    send_chat(message);
  }
  return 0;
}
