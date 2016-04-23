#include "plugin_system.h"
#include "plugin_interface.h"
#include "../protocol/common.h"
#include "list.h"

#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h> // temporary




#ifndef PLUG_DIR
#define PLUG_DIR "./plugins"
#endif
static const char plugin_directory[] = PLUG_DIR;

struct _PluginManager plugin_manager;


#define   PLUG_PREFIX       "init_"
#define   PLUG_PREFIX_LEN   5


void plugin_manager_init(PluginManager *p) 
{
  p->action = new_list();
  p->command = new_list();
  p->plugin = new_list();
}


static char* plugin_path(const char *plugname)
{
  char *path = malloc(4 + strlen(plugin_directory) + strlen(plugname));
  sprintf(path, "%s/%s.so", plugin_directory, plugname);
  return path;
}



static char* plugin_name_init(const char *plugname)
{
  char *init = malloc(1 + PLUG_PREFIX_LEN + strlen(plugname));
  strcpy(init, PLUG_PREFIX);
  strcat(init, plugname);
  return init;
}







int plugin_register(PluginManager *plug_manager, const char *name, Plugin *plug) 
{
  if (mem(plug_manager->plugin, name)) {
    fprintf(stderr, "A plugin called "BOLD "%s" RESET " already exists.\n", name);
    fprintf(stderr, "Plugin registration failed\n");
    return 0;
  }
  // look for collisions with other commands
  for (int i = 0; i < plug->size_command; ++i) 
    if (mem(plug_manager->command, plug->command[i].name)) {
      fprintf(stderr, "A command called "BOLD "%s" RESET " already exists.\n", plug->command[i].name);
      fprintf(stderr, "Plugin registration failed\n");
      return 0;
    }
  // look for collisions with other message actions
  for (int i = 0; i < plug->size_action; ++i) 
    if (mem(plug_manager->action, plug->action[i].name)) {
      fprintf(stderr, "An action called "BOLD "%s" RESET " already exists.\n", plug->action[i].name);
      fprintf(stderr, "Plugin registration failed\n");
      return 0;
    }

  // register plugin
  RegisteredPlugin *plug_reg = malloc(sizeof(RegisteredPlugin));
  plug_reg->plug = plug;
  insert(plug_manager->plugin, name, plug_reg);
  // register commands
  for (int i = 0; i < plug->size_command; ++i) {
    plug_command *pc = malloc(sizeof(plug_command));
    pc->desc = plug->command[i].desc;
    pc->command = plug->command[i].command;
    insert(plug_manager->command, plug->command[i].name, pc);
  }
  // register actions
  for (int i = 0; i < plug->size_action; ++i) {
    plug_action *pa = malloc(sizeof(plug_action));
    pa->desc = plug->action[i].desc;
    pa->action = plug->action[i].action;
    insert(plug_manager->action, plug->action[i].name, pa);
    insert(plug_manager->action, plug->action[i].name, &plug->action[i]);
  }
  return 1;
}



int plugin_unregister(PluginManager *plug_manager, const char *name)
{
  RegisteredPlugin *plug_reg;
  if (!find((void **)&plug_reg, plug_manager->plugin, name)) {
    fprintf(stderr, "Plugin "BOLD "%s" RESET " doesn't exist.\n", name);
    return 0;
  }
  Plugin *p = plug_reg->plug;
  for (int i = 0; i < p->size_command; ++i)
    rm(plug_manager->command, p->command->name);
  for (int i = 0; i < p->size_action; ++i)
    rm(plug_manager->action, p->action->name);
  
  dlclose(plug_reg->lib);
  rm(plug_manager->plugin, name);
  return 1;
}




int loadplugin(PluginManager *plug_manager, const char *plugname)
{
  char *plugpath = plugin_path(plugname);
  void *lib = dlopen(plugpath, RTLD_LAZY);
  free(plugpath);
  if (lib == NULL) {
    fprintf(stderr, "Could not load plugin "BOLD "%s" RESET " from %s.\n", plugname, plugin_directory);
    fprintf(stderr,"Plugin loading failed.\n");
    return 0;
  }
  char *init_name = plugin_name_init(plugname);
  printf("Retrieving init function...\n");
  PluginInitFunc init_func = dlsym(lib, init_name);
  free(init_name);
  if (!init_func) {
    dlclose(lib);
    fprintf(stderr,"Could not load ini function for plugin "BOLD "%s" RESET ".\n", plugname);
    fprintf(stderr,"Plugin loading failed.\n");
    return 0;
  }
  int r = init_func(plug_manager);
  if (r < 0) {
    dlclose(lib);
    fprintf(stderr, "Init function returns %d.\n", r);
    fprintf(stderr, "Plugin loading failed.\n");
    return 0;
  }
  printf("Plugin initialized.\n");
  printf("Finishing plugin registration...\n");
  RegisteredPlugin *plug_reg;
  if (!find((void **)&plug_reg, plug_manager->plugin, plugname)) {
    dlclose(lib);
    fprintf(stderr, RED"Plugin init function hasn't registered the plugin.\n" RESET);
    fprintf(stderr, RED"Plugin loading failed.\n" RESET);
    return 0;
  }
  plug_reg->lib = lib;
  printf("Plugin "BOLD "%s" RESET " now available.\n", plugname);
  return 1;
}



int unloadplugin(PluginManager *plug_manager, const char *plugname)
{
  int r = plugin_unregister(plug_manager, plugname);
  if (r)
      printf("Plugin unloaded.\n");
  return r;
}
