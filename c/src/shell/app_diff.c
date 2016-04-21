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




#define   MSIZE        480
#define   IDAPP_DIFF   "DIFF####"








void action_diff(char *mess, char *content, int lookup_flag) {
  char size[4];
  strncpy(size, content, 3);
  size[3] = 0;
  if (content[3] != ' ' || !isnumeric(size)) {
    debug(RED "action_chat", RED "message content \"%s\" not valid for application.",
        content);
    return;
  }
  if (!lookup_flag)
    sendpacket_all(mess);
}



static void send_chat(char *mess) {
  unsigned len = strlen(mess);
  unsigned size = MSIZE < len ? MSIZE : len;
  char ssize[4];
  itoa(ssize, 4, size);
  sendappmessage_all(IDAPP_DIFF, "%s %s", ssize, mess);
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
  char *line = readline(UNDERLINED "Enter your message:\n" RESET);
  strncpy(message, line, 481);
  free(line);
}


#define FLAG_MESS 1

void cmd_diff(int argc, char **argv)
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
        return;
        break;
      case OPT_MESS:
        flag |= FLAG_MESS;
        strncpy(message, optarg, 481);
        break;
      default:
        usage(argv[0]);
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
}
