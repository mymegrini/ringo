#include "../../plugin_system/plugin_interface.h"
#include "../../plugin_system/protocol_interface.h"
#include "gui.h"

static int cmd_pong(int argc, char **argv);
static int action_pong(const char *message, const char* content, int lookup_flag);

PluginCommand_t pcmd_pong = {
    "pong",
    "Classic multiplayer tennis game running over the network.",
    cmd_pong
};

PluginAction_t paction_pong = {
    "PONG####",
    "Network packet for 'pong' application.",
    action_pong
};

Plugin plugin_pong = {
    1,
    &pcmd_pong,
    1,
    &paction_pong,
    quitPong
};


int init_pong(PluginManager *p)
{
    
    return plugin_register(p, "pong", &plugin_pong);
}




static int cmd_pong(int argc, char **argv){
    
    return launchPong();    
}

static int action_pong(const char *message, const char *content, int lookup_flag)
{
    return 0;
}

