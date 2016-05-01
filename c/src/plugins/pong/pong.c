#include "../../plugin_system/plugin_interface.h"
#include "gui.h"
#include "netcode.h"

PluginCommand_t cmd_pong = {
    "pong",
    "Classic multiplayer tennis game running over the network.",
    launchPong
};

PluginAction_t action_pong = {
    "PONG####",
    "Network packet for 'pong' application.",
    parsePong
};

Plugin plugin_pong = {
    1,
    &cmd_pong,
    1,
    &action_pong,
    quitPong
};


int init_pong(PluginManager *p)
{
    initPong();
    return plugin_register(p, "pong", &plugin_pong);
}


