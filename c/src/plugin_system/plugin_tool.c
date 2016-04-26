#include "plugin_tool.h"
#include "../protocol/common.h"

int (*open_terminal)(pid_t *) = init_outputxterm;
