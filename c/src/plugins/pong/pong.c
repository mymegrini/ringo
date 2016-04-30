#include "../../plugin_system/plugin_interface.h"
#include "../../plugin_system/protocol_interface.h"

static int cmd_pong(int argc, char **argv);
static int action_pong(const char *message, const chat* content, int lookup_flag);
