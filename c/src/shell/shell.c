#include "shell.h"

#include "../common.h"
#include "../protocol/message.h"
#include "../protocol/protocol.h"
#include "../protocol/thread.h"
#include "../list.h"
/* #include "../plugin_system/plugin_system.h" */
#include "../plugin_system/plugin_system.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <signal.h>

#include <pthread.h>
/* #include <readline/readline.h> */
/* #include <readline/history.h> */
#include "../../libreadline/include/readline/readline.h"
#include "../../libreadline/include/readline/history.h"
#include <wordexp.h>

#include <stdio.h>

#include "../banner.h"



////////////////////////////////////////////////////////////////////////////////
// PROTOTYPES
////////////////////////////////////////////////////////////////////////////////

static int cmd_cd(int argc, char **argv);
static int cmd_pwd(int argc, char **argv);

static int cmd_whos(int argc, char **argv);

extern int cmd_gbye(int argc, char **argv);
extern int cmd_exit(int argc, char **argv);
extern int cmd_rinfo(int argc, char **argv);
extern int cmd_info(int argc, char **argv);
extern int cmd_help(int argc, char **argv);
extern int cmd_diff(int argc, char **argv);
extern int cmd_plugin(int argc, char **argv);
extern int cmd_ring(int argc, char **argv);
extern int cmd_trans(int argc, char **argv);
extern int cmd_verbosity(int argc, char **argv);



////////////////////////////////////////////////////////////////////////////////
// VARS
////////////////////////////////////////////////////////////////////////////////

command cmd[] = {
  { "info", "Informations about the program", cmd_info},
  { "cd", "Change working directory.", cmd_cd },
  { "exit", "Quit all rings and exit shell.", cmd_exit},
  { "pwd", "Print working directory.", cmd_pwd},
  { "rdif", "Send messages on the ring.", cmd_diff },
  { "gbye", "Quit a ring.", cmd_gbye },
  { "help", "Show this message.", cmd_help },
  { "rinfo", "Informations on current entity.", cmd_rinfo},
  { "plug", "Add or remove plugins.", cmd_plugin},
  { "ring", "Create, duplicate and join rings.", cmd_ring},
  { "verbosity", "Toggle verbose mode or switch verbosity", cmd_verbosity},
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
  if (getcwd(buff, 1024) == NULL){
      perror("getcwd");
      return;
  }
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
      /* plug_command *pc; */
      /* if (find((void **)&pc, plugin_manager.command, wordx.we_wordv[0])) { */
      /*   r = (pc->command)(wordx.we_wordc, wordx.we_wordv); */
      /*   wordfree(&wordx); */
      /*   return r; */
      /* } */
      r = exec_plugin_command(&plugin_manager, wordx.we_wordc, wordx.we_wordv);
      if (r != -1)
        return r;
      // a little alias for ls...
      if (strcmp(wordx.we_wordv[0], "ls") == 0) {
        char *ls = malloc(strlen(str) + 15);
        sprintf(ls, "%s --color=auto", str);
        if (system(ls));
        free(ls);
      }
      else {
        if(system(str))
          r = 0;
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
  rl_catch_signals = 0;
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
    iter = get_iterator(get_commandlist());
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
    while (iter && (name = iterator_getname(iter)))
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


extern void exit_properly(void);



static void signal_handler_exit(int signum)
{
  exit_properly();
}



static void signal_handler_restart_mdiff(int signum)
{
  debug("SIGUSR1 handler", "calling restart_mdiffmanager");
  restart_mdiffmanager();
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
  signal(SIGUSR1, signal_handler_restart_mdiff);

  if ((homedir = getenv("HOME")) == NULL)
    homedir = getpwuid(getuid())->pw_dir;

  if (read_history(HISTORY_FILE) != 0) {
    FILE *fp = fopen(HISTORY_FILE, "ab+");
    fclose(fp);
  }
}

static void welcome_message() {
  printf("%s", banner[2]);
}

////////////////////////////////////////////////////////////////////////////////
// GLOBAL
////////////////////////////////////////////////////////////////////////////////

void *run_shell(void *nothing) {

  init_shell();
  actualize_prompt();
  initialize_readline();

  welcome_message();

  char *line = NULL;
  char *old_line = NULL;
  line = readline(prompt);
  old_line = strdup(line);
  add_history(line);
  append_history(1, HISTORY_FILE);

  while(1) {
    if (line && *line) {
      if (strcmp(line, old_line) != 0) {
        add_history(line);
        append_history(1, HISTORY_FILE);
        free(old_line);
        old_line = strdup(line);
      }
      exec_cmd(line);
    }
    if (line) {
      free(line);
      line = NULL;
    }
    line = readline(prompt);
  }
  return NULL;
}



