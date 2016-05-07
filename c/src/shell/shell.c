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
#include <sys/types.h>
#include <pwd.h>
#include <signal.h>

#include <pthread.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <wordexp.h>

#include <stdio.h>




////////////////////////////////////////////////////////////////////////////////
// PROTOTYPES
////////////////////////////////////////////////////////////////////////////////

static int cmd_cd(int argc, char **argv);
static int cmd_pwd(int argc, char **argv);

static int cmd_whos(int argc, char **argv);

extern int cmd_gbye(int argc, char **argv);
extern int cmd_exit(int argc, char **argv);
extern int cmd_info(int argc, char **argv);
extern int cmd_help(int argc, char **argv);
extern int cmd_diff(int argc, char **argv);
extern int cmd_plugin(int argc, char **argv);
extern int cmd_ring(int argc, char **argv);



////////////////////////////////////////////////////////////////////////////////
// VARS
////////////////////////////////////////////////////////////////////////////////

command cmd[] = {
  { "cd", "Change working directory.", cmd_cd },
  { "exit", "Quit all rings and exit shell.", cmd_exit},
  { "pwd", "Print working directory.", cmd_pwd},
  { "rdif", "Send messages on the ring.", cmd_diff },
  { "gbye", "Quit a ring.", cmd_gbye },
  { "help", "Show this message.", cmd_help },
  { "info", "Display informations on current entity.", cmd_info},
  { "plug", "Add or remove plugins.", cmd_plugin},
  { "ring", "Create, duplicate and join rings.", cmd_ring},
  { "whos", "Getting to know each other...", cmd_whos },
  { NULL, NULL, NULL }
};


static const char *homedir;
static char prompt[260];
////////////////////////////////////////////////////////////////////////////////
// LOCAL
////////////////////////////////////////////////////////////////////////////////
static void extract_dir_from_path(char *dir, const char *path)
{
  int len;
  for (len = strlen(path) - 1; path[len] != '/'; --len)
    ;
  if (len == 0)
    *dir = '/';
  else
    strcpy(dir, path+len+1);
}



static void actualize_prompt()
{
  char buff[1024];
  getcwd(buff, 1024);
  char dir[255];
  extract_dir_from_path(dir, buff);
  sprintf(prompt, BOLD GREEN "[" CYAN "%s" GREEN "]" YELLOW " $> " RESET, dir);
}



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
      for (int i = 0; cmd[i].name; i++) {
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
      // a little alias for ls...
      if (strcmp(wordx.we_wordv[0], "ls") == 0) {
        char *ls = malloc(strlen(str) + 15);
        sprintf(ls, "%s --color=auto", str);
        system(ls);
        free(ls);
      }
      else {
        system(str);
      }
      break;
  }
  wordfree(&wordx);
  return r;
}



static int cmd_cd(int argc, char **argv)
{
  if (argc > 2) {
    fprintf(stderr, "Too many arguments.\n");
    return 1;
  }
  const char *dir = argc == 1 ? homedir : argv[1];
  if (chdir(dir) == -1) {
    perror(dir);
    return 1;
  }
  actualize_prompt();
  return 0;
}



static int cmd_pwd(int argc, char **argv) 
{
  if (argc != 1) {
    fprintf(stderr, "Too many arguments.\n");
    return 1;
  }

  char buff[1024];
  if (getcwd(buff, 1024) == NULL) {
    perror(NULL);
    return 1;
  }
  printf("%s\n", buff);
  return 0;
}


static int cmd_whos(int argc, char **argv) {
  if (argc != 1) {
    fprintf(stderr, "Usage:\t%s", argv[1]);
    return 1;
  }
  sendmessage_all("WHOS", "");
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// COMPLETION
////////////////////////////////////////////////////////////////////////////////

char *command_generator (char *text, int state);
char **ringo_completion (const char *text, int start, int end);
char **completion_matches (const char *text, char *(*command_generator)(char *, int));

/* Tell the GNU Readline library how to complete.  We want to try to complete
   on command names if this is the first word in the line, or on filenames
   if not. */
void initialize_readline ()
{
  /* Allow conditional parsing of the ~/.inputrc file. */
  rl_readline_name = "Ringo";

  /* Tell the completer that we want a crack first. */
  rl_attempted_completion_function = ringo_completion;
}

/* Attempt to complete on the contents of TEXT.  START and END show the
   region of TEXT that contains the word to complete.  We can use the
   entire line in case we want to do some simple parsing.  Return the
   array of matches, or NULL if there aren't any. */
char **ringo_completion (const char *text, int start, int end)
{
  char **matches;

  matches = (char **)NULL;

  /* If this word is at the start of the line, then it is a command
     to complete.  Otherwise it is the name of a file in the current
     directory. */
  if (start == 0)
    matches = completion_matches (text, command_generator);

  return (matches);
}

/* Generator function for command completion.  STATE lets us know whether
   to start from scratch; without any state (i.e. STATE == 0), then we
   start at the top of the list. */
char *command_generator (char *text, int state)
{
  static int list_index, len;
  static iterator iter;
  char *name;

  /* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
  if (!state)
  {
    list_index = 0;
    iter = get_iterator(plugin_manager.command);
    len = strlen (text);
  }

  /* Return the next name which partially matches from the command list. */
  while ((name = cmd[list_index].name))
  {
    list_index++;

    if (strncmp (name, text, len) == 0)
      return strdup(name);
  }

  if (iter) {
    while ((name = iterator_getname(iter)))
    {
      iterate(&iter);

      if (strncmp (name, text, len) == 0)
        return strdup(name);
    }
  }

  /* If no names matched, then return NULL. */
  return ((char *)NULL);
}

//// END OF COMPLETION



static void signal_handler_exit(int signum)
{
  cmd_exit(0, NULL);
}



static void signal_handler_empty(int signum)
{
  return ;
}



#ifndef HISTORY_FILE
#define HISTORY_FILE NULL
#endif

static void init_shell()
{
  signal(SIGINT, signal_handler_exit);
  signal(SIGTERM, signal_handler_exit);
  signal(SIGQUIT, signal_handler_exit);
  signal(SIGUSR1, signal_handler_empty);

  if ((homedir = getenv("HOME")) == NULL)
    homedir = getpwuid(getuid())->pw_dir;

  if (read_history(HISTORY_FILE) != 0) {
    debug("init_shell", "cannot read history from %s", HISTORY_FILE);
    FILE *fp = fopen(HISTORY_FILE, "ab+");
    fclose(fp);
  }
}

////////////////////////////////////////////////////////////////////////////////
// GLOBAL
////////////////////////////////////////////////////////////////////////////////

void run_shell() {

  init_shell();
  actualize_prompt();
  initialize_readline();

  char *line = NULL;
  while(1) {
    if (line) {
      free(line);
      line = NULL;
    }
    line = readline(prompt);
    if (line && *line) {
      add_history(line);
      append_history(1, HISTORY_FILE);
      exec_cmd(line);
    }
  }
}



