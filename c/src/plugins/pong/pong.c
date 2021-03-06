#include "../../plugin_system/plugin_programmer_interface.h"
#include "pong.h"
#include "gui.h"
#include "netcode.h"
#include "engine.h"


PluginCommand_t cmd_pong = {
    "pong",
    "Classic multiplayer tennis game running over the network.",
    launchPong
};

PluginAction_t action_pong = {
    PONG_TYPE,
    "Network packet handling for 'pong' application.",
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
    initEngine();
    return plugin_register(p, "pong", &plugin_pong);
}
