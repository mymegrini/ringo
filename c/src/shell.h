#ifndef INTERFACE_H
#define INTERFACE_H

typedef struct cmd {
    char name[32];
    char desc[512];
    void (*exec)(int argc, char **args);
} command;




void exec_cmd(char *str);



#endif /* INTERFACE_H */
