#ifndef PLUGIN_SYSTEM_H
#define PLUGIN_SYSTEM_H

#include "list.h"
#include "plugin_interface.h"

typedef struct _PluginManager PluginManager;
extern PluginManager plugin_manager;



typedef struct RegisteredPlugin {
  Plugin *plug;
  void *lib;
} RegisteredPlugin;



struct _PluginManager
{
  list action;
  list command;
  list plugin;
};


typedef struct plug_action {
  char *desc;
  PluginAction action;
} plug_action;

typedef struct plug_command {
  char *desc;
  PluginCommand command;
} plug_command;



void plugin_manager_init(PluginManager *p);
int loadplugin(PluginManager *plug_manager, const char *plugname);
int unloadplugin(PluginManager *plug_manager, const char *plugname);

#endif /* PLUGIN_SYSTEM_H */
