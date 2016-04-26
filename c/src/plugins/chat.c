#include "../protocol/common.h"
#include "../plugin_system/plugin_interface.h"
#include "../plugin_system/protocol_interface.h"

#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>


////////////////////////////////////////////////////////////////////////////////
// GLOBALS
////////////////////////////////////////////////////////////////////////////////
static void getmessage(char *message);
static void help(char *argv0);
static void usage(char *argv0);
static void send_chat(const char *mess);
static void print_chat(const char *name, const char *content);
static void unpadstrn(char *unpadded, const char *str, int len);
static void unpadstr(char *unpadded, const char *str);
static void padstr(char *padded, const char *str, int size);


#define   MSIZE        485
#define   IDAPP_CHAT   "CHAT####"


static int chat_fd = STDOUT_FILENO;

//// END OF GLOBALS




////////////////////////////////////////////////////////////////////////////////
// PLUGIN DATAS
////////////////////////////////////////////////////////////////////////////////

#define CHAT_TYPE "CHAT####"

static int cmd_chat(int argc, char **argv);
static int action_chat(const char *message, const char *content, int lookup_flag);



PluginCommand_t pcmd_chat = {
  "chat",
  "pluging's chat command",
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
  &paction_chat
};



int init_chat(PluginManager *p)
{
  return plugin_register(p, "chat", &plug_chat);
}



int action_chat(const char *mess, const char *content, int lookup_flag) {

  if (lookup_flag)
    return 1;

  if (content[3] != ' ' || !isnumericn(content, 3) ||
      content[12] != ' '){
    debug(RED "action_chat", RED "message \"%s\" not valid for application.",
        mess);
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
  char ssize[4];
  itoa(ssize, 4, size);
  char name[9];
  padstr(name, info->id, 8);
  send_message(IDAPP_CHAT, "%s %s %s", ssize, name, mess);
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



static void getmessage(char *message)
{
  char *line = readline(BOLD "Enter your message:\n" RESET);
  strncpy(message, line, 481);
  free(line);
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
  debug("unpadstr", GREEN "str: \"%s\"", str);
  while (*str == ' ' && --len >= 0)
    ++str;
  strncpy(unpadded, str, len);
  unpadded[len] = 0;
}

//// END OF TOOLS

