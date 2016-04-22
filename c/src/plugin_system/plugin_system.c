#include "plugin_system.h"
#include "plugin_interface.h"
#include "../protocol/common.h"
#include "list.h"

#include <string.h>
#include <stdlib.h>
#include <search.h>
#include <dlfcn.h>
#include <unistd.h> // temporary



struct _PluginManager plugin_manager;

static const char plugin_directory[] = "./plugins";

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
    fprintf(stderr, RED "A plugin called %s already exists.\n", name);
    fprintf(stderr, RED "Plugin registration failed\n");
    return 0;
  }
  // look for collisions with other commands
  for (int i = 0; i < plug->size_command; ++i) 
    if (mem(plug_manager->command, plug->command[i].name)) {
      fprintf(stderr, RED "A command called %s already exists.\n", plug->command[i].name);
      fprintf(stderr, RED "Plugin registration failed\n");
      return 0;
    }
  // look for collisions with other message actions
  for (int i = 0; i < plug->size_action; ++i) 
    if (mem(plug_manager->action, plug->action[i].name)) {
      fprintf(stderr, RED "An action called %s already exists.\n", plug->action[i].name);
      fprintf(stderr, RED "Plugin registration failed\n");
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
    fprintf(stderr, RED "Plugin %s doesn't exist.\n", name);
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
    fprintf(stderr, RED"Could not load plugin %s from %s.\n" RESET, plugname, plugin_directory);
    fprintf(stderr, RED"Plugin loading failed.\n" RESET);
    return 0;
  }
  char *init_name = plugin_name_init(plugname);
  printf("Retrieving plugin init function...\n");
  PluginInitFunc init_func = dlsym(lib, init_name);
  free(init_name);
  if (!init_func) {
    dlclose(lib);
    fprintf(stderr, RED"Could not load ini function for plugin %s.\n" RESET, plugname);
    fprintf(stderr, RED"Plugin loading failed.\n" RESET);
    return 0;
  }
  int r = init_func(plug_manager);
  if (r < 0) {
    dlclose(lib);
    fprintf(stderr, RED "Init function returns %d.\n" RESET, r);
    fprintf(stderr, RED"Plugin loading failed.\n" RESET);
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
  printf("Plugin registered.\n");
  return 1;
}



int unloadplugin(PluginManager *plug_manager, const char *plugname)
{
  int r = plugin_unregister(plug_manager, plugname);
  if (r)
      printf("Plugin unloaded.\n");
  return r;
}
