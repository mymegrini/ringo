#include "shell.h"

#include "../protocol/common.h"
#include "../protocol/message.h"
#include "../protocol/protocol.h"
#include "../protocol/thread.h"

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

static void cmd_whos(int argc, char **argv);
extern void cmd_gbye(int argc, char **argv);
extern void cmd_info(int argc, char **argv);
extern void cmd_help(int argc, char **argv);




// TEST
void echo(int argc, char **argv) {
    
    for (++argv; *argv != NULL; ++argv)
        printf("%s ", *argv);
    printf("\n");
}

////////////////////////////////////////////////////////////////////////////////
// VARS
////////////////////////////////////////////////////////////////////////////////

command cmd[] = { 
    { "echo", "Print a message", echo },
    { "whos", "Getting to know each other...", cmd_whos },
    { "gbye", "Quit a ring.", cmd_gbye },
    { "info", "Display informations on current entity.", cmd_info},
    { "help", "Show this message.", cmd_help },
    { "", "", NULL }
};

extern volatile int nring;

////////////////////////////////////////////////////////////////////////////////
// LOCAL
////////////////////////////////////////////////////////////////////////////////


static void exec_cmd(const char *str) {
    wordexp_t wordx;
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
            for (int i = 0; cmd[i].name[0] != 0; i++) {
                if (strcmp(cmd[i].name, wordx.we_wordv[0]) == 0) {
                    (*cmd[i].exec)(wordx.we_wordc, wordx.we_wordv);
                    wordfree(&wordx);
                    return;
                }
            }
            break;
    }
}

/*
 *static void prompt() {
 *    char dirname[256];
 *    if (!getcwd(dirname, 256)) return;
 *    printf("%s > ", dirname);
 *}
 */


static void cmd_whos(int argc, char **argv) {
    if (argc != 1) {
        fprintf(stderr, "Usage:\t%s", argv[1]);
        return ;
    }
    sendmessage_all("WHOS", "");
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



