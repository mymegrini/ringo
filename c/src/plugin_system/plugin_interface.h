#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

typedef struct _PluginManager PluginManager;

extern PluginManager plugin_manager;



typedef int (*PluginAction)(const char *, const char *, int); // action to do, take the message, content and lookup_flag
typedef int (*PluginCommand)(int, char **); // plugin command
typedef int (*PluginInitFunc)(PluginManager*);



typedef struct PluginAction_t {
  char         name[9];
  char         desc[512];
  PluginAction action;
} PluginAction_t;



typedef struct PluginCommand_t {
  char          name[30];
  char          desc[512];
  PluginCommand command;
} PluginCommand_t;



typedef struct Plugin {
  int              size_command;
  PluginCommand_t *command;
  int              size_action;
  PluginAction_t  *action;
} Plugin;



extern int plugin_register(PluginManager *plug_manager, const char *name, Plugin *plug);


#endif /* PLUGIN_INTERFACE_H */
