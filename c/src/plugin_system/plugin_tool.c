#include "plugin_tool.h"
#include "../common.h"

int (*open_terminal)(pid_t *) = init_outputxterm;
int (*open_terminal_communication)(pid_t *, int *) = init_xterm_communication;

