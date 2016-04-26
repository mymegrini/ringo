#include "shell.h"

#include "../protocol/common.h"
#include "../protocol/message.h"
#include "../protocol/protocol.h"
#include "../protocol/thread.h"
#include "../plugin_system/list.h"
#include "../plugin_system/plugin_system.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <wordexp.h>

#include <stdio.h>




////////////////////////////////////////////////////////////////////////////////
// PROTOTYPES
////////////////////////////////////////////////////////////////////////////////

static int cmd_whos(int argc, char **argv);
extern int cmd_gbye(int argc, char **argv);
extern int cmd_info(int argc, char **argv);
extern int cmd_help(int argc, char **argv);
extern int cmd_diff(int argc, char **argv);
extern int cmd_plugin(int argc, char **argv);




////////////////////////////////////////////////////////////////////////////////
// VARS
////////////////////////////////////////////////////////////////////////////////

command cmd[] = {
  { "rdif", "Send messages on the ring.", cmd_diff },
  { "gbye", "Quit a ring.", cmd_gbye },
  { "help", "Show this message.", cmd_help },
  { "info", "Display informations on current entity.", cmd_info},
  { "plug", "Add or remove plugins.", cmd_plugin},
  { "whos", "Getting to know each other...", cmd_whos },
  { "", "", NULL }
};

extern volatile int nring;

////////////////////////////////////////////////////////////////////////////////
// LOCAL
////////////////////////////////////////////////////////////////////////////////


static int exec_cmd(const char *str) {
  wordexp_t wordx;
  int r = -1;
  switch (wordexp(str, &wordx, 0)) {
    case WRDE_BADCHAR:
      fprintf(stderr, 
          "Illegal  occurrence of newline or one of "
          "|, &, ;, <, >, (, ), {, }.");
      break;
    case WRDE_BADVAL:
      break;
    case WRDE_CMDSUB:
      break;
    case WRDE_NOSPACE:
      break;
    case WRDE_SYNTAX:
      fprintf(stderr, "Syntax error.\n");
      break;
    default:
      // look for command
      for (int i = 0; cmd[i].name[0] != 0; i++) {
        if (strcmp(cmd[i].name, wordx.we_wordv[0]) == 0) {
          r = (*cmd[i].exec)(wordx.we_wordc, wordx.we_wordv);
          wordfree(&wordx);
          return r;
        }
      }
      // look for plugin command
      plug_command *pc;
      if (find((void **)&pc, plugin_manager.command, wordx.we_wordv[0])) {
        r = (pc->command)(wordx.we_wordc, wordx.we_wordv);
        wordfree(&wordx);
        return r;
      }
      system(str);
      break;
  }
  wordfree(&wordx);
  return r;
}

/*
 *static void prompt() {
 *    char dirname[256];
 *    if (!getcwd(dirname, 256)) return;
 *    printf("%s > ", dirname);
 *}
 */


static int cmd_whos(int argc, char **argv) {
  if (argc != 1) {
    fprintf(stderr, "Usage:\t%s", argv[1]);
    return 1;
  }
  sendmessage_all("WHOS", "");
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// GLOBAL
////////////////////////////////////////////////////////////////////////////////

void run_shell() {
  char *line = NULL;
  while(1) {
    /*
     *prompt();
     *char *line = NULL;
     *size_t size = 0;
     *size = getline(&line, &size, stdin);
     *line[size-1] = 0;
     */
    if (line) {
      free(line);
      line = NULL;
    }
    line = readline("$> ");
    if (line && *line)
      exec_cmd(line);
  }
}


