#include "cmd_help.h"
#include "shell.h"
#include "../protocol/protocol.h"
#include "../protocol/message.h"
#include "../protocol/common.h"
#include "../protocol/network.h"
#include "../protocol/thread.h"


extern command cmd[];

static void print_cmd(command *c) {
  printf(UNDERLINED BOLD GREEN "%s\n" RESET
      YELLOW "%s\n" RESET,
      c->name, c->desc);
}

int cmd_help(int argc, char *argv[])
{
  for (int i = 0; *cmd[i].name; ++i)
    print_cmd(&cmd[i]);
  return 0;
}

