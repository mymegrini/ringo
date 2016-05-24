#include "plugin_system.h"
#include "plugin_programmer_interface.h"
/* #include "../common.h" */
#include "../list.h"

#include <dlfcn.h>
#include <unistd.h> // temporary


#include <pthread.h>



#ifndef PLUG_DIR
#define PLUG_DIR "./plugins"
#endif

char default_plugin_directory[] = PLUG_DIR;
char *plugin_directory          = default_plugin_directory;


struct request {
  list l;
  pthread_mutex_t mutex;
  pthread_cond_t  can_pop_cond;
};



typedef struct RegisteredPlugin {
  Plugin *plug;
  void *lib;
} RegisteredPlugin;



struct _PluginManager
{
  list action;
  list command;
  list plugin;
  struct request request;
};



struct _PluginManager plugin_manager;


#define   PLUG_PREFIX       "init_"
#define   PLUG_PREFIX_LEN   5


void plugin_manager_init(PluginManager *p) 
{
  p->action    = new_list();
  p->command   = new_list();
  p->plugin    = new_list();
  p->request.l = new_list();

  pthread_cond_init(&p->request.can_pop_cond, 0);
  pthread_mutex_init(&p->request.mutex, 0);
}


static char* plugin_path(const char *plug_dir, const char *plugname)
{
  char *path = malloc(4 + strlen(plug_dir) + strlen(plugname));
  sprintf(path, "%s/%s.so", plug_dir, plugname);
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
  // call closing function
  if (p->close_plugin)
    p->close_plugin();
  
  dlclose(plug_reg->lib);
  rm(plug_manager->plugin, name);
  return 1;
}




int loadplugin(PluginManager *plug_manager, const char *plug_dir, const char *plugname)
{
  printf(UNDERLINED "Loading plugin " BOLD "%s" RESET UNDERLINED ":" RESET "\n", plugname);
  char *plugpath = plugin_path(plug_dir, plugname);
  void *lib = dlopen(plugpath, RTLD_LAZY);
  free(plugpath);
  if (lib == NULL) {
    fprintf(stderr, "\tCould not open plugin "BOLD "%s" RESET " from %s.\n%s\n",
	    plugname, plug_dir, dlerror());
    fprintf(stderr,"Plugin loading failed.\n");
    return 0;
  }
  char *init_name = plugin_name_init(plugname);
  printf("\tRetrieving init function...\n");
  PluginInitFunc init_func = dlsym(lib, init_name);
  free(init_name);
  if (!init_func) {
    dlclose(lib);
    fprintf(stderr,"Could not load init function for plugin "BOLD "%s" RESET ".\n", plugname);
    fprintf(stderr,"Plugin loading failed.\n");
    return 0;
  }
  if (!init_func(plug_manager)){
      dlclose(lib);
      fprintf(stderr, "Plugin loading failed.\n");
      return 0;
  }
  printf("\tFinishing plugin registration...\n");
  RegisteredPlugin *plug_reg;
  if (!find((void **)&plug_reg, plug_manager->plugin, plugname)) {
    dlclose(lib);
    fprintf(stderr, RED"Plugin init function hasn't registered the plugin.\n" RESET);
    fprintf(stderr, RED"Plugin loading failed.\n" RESET);
    return 0;
  }
  plug_reg->lib = lib;
  printf("Plugin "BOLD "%s" RESET " now available.\n\n", plugname);
  return 1;
}



int unloadplugin(PluginManager *plug_manager, const char *plugname)
{
  char *name = strdup(plugname);
  int r = plugin_unregister(plug_manager, plugname);
  if (r)
      printf("Plugin %s closed.\n", name);
  free(name);
  return r;
}



int plugin_extract_name(char **name, const char *plug)
{
  int len = strlen(plug);
  if (len > 3 && strcmp(".so", plug+len-3) == 0)
  {
    *name = strndup(plug, len-3);
    return 1;
  }
  return 0;
}



#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

int load_all_plugins(PluginManager *plug_manager, const char *plug_dir)
{
  DIR *dir;
  if ((dir = opendir (plug_dir)) != NULL) {
    /* print all the files and directories within directory */
    struct dirent *ent;
    char wd[256];
    if (getcwd(wd, 256) == NULL){
	perror("load_all_plugins");
	return 1;
    }
    if (chdir(plug_dir)){
	perror("load_all_plugins");
	return 1;
    }
    while ((ent = readdir (dir)) != NULL) {
      struct stat info;
      /* debug("load_all_plugins", "file: %s, S_ISREG: %d, S_ISDIR: %d", ent->d_name, S_ISREG(info.st_mode), S_ISDIR(info.st_mode)); */
      if (! S_ISDIR(info.st_mode)) {
        char *plugname;
        if (plugin_extract_name(&plugname, ent->d_name)) {
          loadplugin(plug_manager, plug_dir, plugname);
          free(plugname);
        }
      }
    }
    if(chdir(wd)){
	perror("load_all_plugins");
	return 1;
    }
    closedir(dir);
    return 1;
  }
  else {
    fprintf(stderr, "Can't open directory %s.\n", plug_dir);
    return 0;
  }
}


int unload_all_plugins(PluginManager *plug_manager)
{
  int r = 1;
  for (iterator i = get_iterator(plug_manager->plugin); i != NULL; i = get_iterator(plug_manager->plugin)) {
    r &= unloadplugin(plug_manager, iterator_getname(i));
  }
  return r;
}



int exec_plugin_command(PluginManager *plug_manager, int argc, char *argv[])
{
  plug_command *pc;
  if (find((void **)&pc, plug_manager->command, argv[0])) {
    return (pc->command)(argc, argv);
  }
  return -1;

}



int exec_plugin_action(PluginManager *plug_manager, const char *idapp,
    const char *message, const char* content, int lookup_flag)
{
  plug_action *pa;
  if (find((void **)&pa, plug_manager->action, idapp)) {
    return pa->action(message, content, lookup_flag);
  }
  return -1;
}



////////////////////////////////////////////////////////////////////////////////
// MESSAGE REQUESTING
////////////////////////////////////////////////////////////////////////////////

void push_message(PluginManager *plug_manager, const char *message)
{ 
  pthread_mutex_lock(&plug_manager->request.mutex);
  insert(plug_manager->request.l, message, NULL);
  pthread_cond_signal(&plug_manager->request.can_pop_cond);
  pthread_mutex_unlock(&plug_manager->request.mutex);
}



static char *pop_message(PluginManager *plug_manager)
{
  char *msg = NULL;
  pthread_mutex_lock(&plug_manager->request.mutex);
  while ((msg = pop_name(plug_manager->request.l)) == NULL)
    pthread_cond_wait(&plug_manager->request.can_pop_cond,
        &plug_manager->request.mutex);
  pthread_mutex_unlock(&plug_manager->request.mutex);
  return msg;
}



const char *get_message()
{
  return pop_message(&plugin_manager);
}


//// END OF MESSAGE REQUESTING

////////////////////////////////////////////////////////////////////////////////
// TOOL
////////////////////////////////////////////////////////////////////////////////


static void print_action(PluginAction_t *action)
{
  printf(BOLD "%s:\t" RESET "%s\n", action->name, action->desc);
}



static void print_command(PluginCommand_t *command)
{
  printf(BOLD "%s:\t" RESET "%s\n", command->name, command->desc);
}



static void print_registeredplugin(char *name, void *data)
{
  RegisteredPlugin *plug_reg = (RegisteredPlugin *)data;
  Plugin           *p        = plug_reg->plug;

  printf(BOLD UNDERLINED "%s" RESET "\n", name);
  printf(UNDERLINED "commands:" RESET "\n");
  if (p->size_command)
    for (int i = 0; i < p->size_command; ++i)
      print_command(p->command+i);
  else
    printf("No command\n");
  printf(UNDERLINED "actions:" RESET "\n");
  if (p->size_action)
    for (int i = 0; i < p->size_action; ++i)
      print_action(p->action+i);
  else
    printf("No action.\n");
}



static void display_plugins(PluginManager *p)
{
  printf(UNDERLINED BOLD "Plugins:" RESET "\n");
  iter(p->plugin, print_registeredplugin);
}



void show_plugins()
{
  display_plugins(&plugin_manager);
}



list get_commandlist()
{
  return plugin_manager.command;
}



//// END OF TOOL
