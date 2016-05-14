#ifndef PLUGIN_TOOL_H
#define PLUGIN_TOOL_H

#include <unistd.h>

extern int (*open_terminal)(pid_t *pid);
extern int (*open_terminal_communication)(pid_t *pid, int *pipe);

extern int isnumericn(const char *str, int n);

extern void itoa(char *s, int size, int i);

extern char *readline (const char *prompt);

extern void (*verbose)(char *format, ...);

#include "list.h"

#endif /* PLUGIN_TOOL_H */
