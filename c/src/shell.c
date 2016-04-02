#include "shell.h"

#include "common.h"
#include "message.h"
#include "protocol.h"
#include "thread.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>

#include <stdio.h>



////////////////////////////////////////////////////////////////////////////////
// PROTOTYPES
////////////////////////////////////////////////////////////////////////////////

static void cmd_whos(int argc, char **argv);
static void cmd_gbye(int argc, char **argv);


////////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////////

typedef struct cmd {
    char name[32];
    char desc[512];
    void (*exec)(int argc, char **args);
} command;


// TEST
void echo(int argc, char **argv) {
    
    for ( ++argv; *argv != NULL; ++argv)
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
    { "", "", NULL }
};

extern int nring;
extern entity ent;
extern int wait_goodbye;

////////////////////////////////////////////////////////////////////////////////
// LOCAL
////////////////////////////////////////////////////////////////////////////////

static char **split(char *str) {
    char **res = NULL;
    char *cpy = strdup(str);
    char *token = strtok(cpy, " ");
    int n = 0;
   while (token != NULL) {
       res = realloc(res, sizeof(char *) * ++n);
       if (res == NULL)
           return NULL;
       res[n-1] = (char *)malloc(strlen(token) + 1);
       strcpy(res[n-1], token);
       token = strtok(NULL, " ");
   } 
   res = realloc(res, sizeof(char *) * (n+1));
   res[n] = NULL;
   free(cpy);
   return res;
}



static void free_split(char **sp) {
    char **i;
    for (i = sp; *i != NULL; i++ )
        free(*i);
    free(sp);
}



static void exec_cmd(char *str) {
    char **sp = split(str);
    if (sp == NULL) {
#ifdef DEBUG
        debug("exec_cmd(\"%s\")", "split returned NULL. realloc error.\n", str);
#endif
        free_split(sp);
        return;
    }
    if (*sp == NULL) {
        free_split(sp);
        return;
    }
    int argc;
    for (argc = 1; sp[argc] != NULL; ++argc)
        ;
#ifdef DEBUG
    debug("exec_cmd(str)", "Looking for \"%s\" command", sp[0]);
#endif
    for (int i = 0; cmd[i].name[0] != 0; i++) {
        if (strcmp(cmd[i].name, sp[0]) == 0) {
#ifdef DEBUG
            debug("exec_cmd(str)", "Command found.");
#endif
            (*cmd[i].exec)(argc, sp);
            free_split(sp);
            return;
        }
    }
#ifdef DEBUG
    debug("exec_cmd(str)", "Command not found.");
#endif
    system(str);
}


static void prompt() {
    char dirname[256];
    getcwd(dirname, 256);
    printf("%s > ", dirname);
}


static void cmd_whos(int argc, char **argv) {
    if (argc != 1) {
        fprintf(stderr, "Usage:\t%s", argv[1]);
        return ;
    }
    sendmessage_all("WHOS", "");
}

// TODO add parameter to choose the ring
static void cmd_gbye(int argc, char **argv) {
    if (argc != 1) {
        fprintf(stderr, "Usage:\t%s", argv[1]);
        return;
    }
    close_tcpserver();
    char *udp = itoa4(ent.udp);
    char *port_next = itoa4(ent.port_next[nring]);
    sendmessage_all("GBYE", "%s %s %s %s", ent.ip_self, udp, ent.ip_next[nring],
            port_next);
    wait_goodbye = 1;
    verbose("Waiting for EYBG message...");
}
////////////////////////////////////////////////////////////////////////////////
// GLOBAL
////////////////////////////////////////////////////////////////////////////////

void run_shell() {
    while(1) {
        prompt();
        char *line = NULL;
        size_t size = 0;
        size = getline(&line, &size, stdin);
        line[size-1] = 0;
        exec_cmd(line);
    }
}



