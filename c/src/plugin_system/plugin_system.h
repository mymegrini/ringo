#ifndef PLUGIN_SYSTEM_H
#define PLUGIN_SYSTEM_H

#include "../list.h"
#include <stdarg.h>
#include <stdlib.h>
#include "plugin_programmer_interface.h"
#include "../common.h"

typedef struct _PluginManager PluginManager;


extern PluginManager plugin_manager;
extern char *plugin_directory;





typedef struct plug_action {
  char *desc;
  PluginAction action;
} plug_action;

typedef struct plug_command {
  char *desc;
  PluginCommand command;
} plug_command;



void plugin_manager_init(PluginManager *p);
int loadplugin(PluginManager *plug_manager, const char *plug_dir, const char *plugname);
int unloadplugin(PluginManager *plug_manager, const char *plugname);
int load_all_plugins(PluginManager *plug_manager, const char *plug_dir);
int unload_all_plugins(PluginManager *plug_manager);
int plugin_extract_name(char **name, const char *plug);

int exec_plugin_command(PluginManager *plug_manager, int argc, char *argv[]);
int exec_plugin_action(PluginManager *plug_manager, const char *idapp,
    const char *message, const char* content, int lookup_flag);

const char *get_message();


#ifdef JAVA_PLUGIN_SYSTEM
#include <jni.h>
struct PluginManagerEnv {
  JNIEnv *env;
  jobject obj;
};

extern struct PluginManagerEnv env;
#endif

////////////////////////////////////////////////////////////////////////////////
// TOOL
////////////////////////////////////////////////////////////////////////////////
list get_commandlist();
void show_plugins();

//// END OF TOOL
#endif /* PLUGIN_SYSTEM_H */
