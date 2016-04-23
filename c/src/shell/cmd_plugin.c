#include "../plugin_system/plugin_system.h"
#include "../protocol/common.h"

#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>





static void usage(char *argv0);
static void help(char *argv0);
static void show_plugins(PluginManager *p);


#define   OPT_HELP      'h'
#define   OPTL_HELP     "help"
#define   OPT_LOAD      'l'
#define   OPTL_LOAD     "load"
#define   OPT_UNLOAD    'u'
#define   OPTL_UNLOAD   "unload"

#define   OPT_STRING    "hl:u:"



static struct option longopts[] = {
  {OPTL_HELP,     no_argument,         0,   OPT_HELP},
  {OPTL_LOAD,     required_argument,   0,   OPT_LOAD},
  {OPTL_UNLOAD,   required_argument,   0,   OPT_UNLOAD},
  {0,             0,                   0,   0}
};





#define   FLAG_LOAD     1
#define   FLAG_UNLOAD   2



int cmd_plugin(int argc, char **argv)
{
  char *loadplug, *unloadplug;
  int c, indexptr;
  short flag = 0;
  optind = 0;
  while ((c = getopt_long(argc, argv, OPT_STRING,
          longopts, &indexptr)) != -1) {
    switch (c) {
      case OPT_HELP:
        help(argv[0]);
        return 0;
        break;
      case OPT_LOAD:
        flag |= FLAG_LOAD;
        loadplug = strdup(optarg);
        break;
      case OPT_UNLOAD:
        flag |= FLAG_UNLOAD;
        unloadplug = strdup(optarg);
        break;
      default:
        usage(argv[0]);
        return 1;
        break;
    }
  }
  if (flag == 0)
    show_plugins(&plugin_manager);
  else if (flag & FLAG_UNLOAD) {
    if (!unloadplugin(&plugin_manager, unloadplug))
      return 1;
  }
  else if (flag & FLAG_LOAD) {
    if (!loadplugin(&plugin_manager, loadplug))
      return 1;
  }
  return 1;
}



static void usage(char *argv0)
{
  printf("Usage:\t%s [-u plugins] [-lplugins] [-h]\n", argv0);
}



static void help(char *argv0)
{
  usage(argv0);
}



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



static void show_plugins(PluginManager *p)
{
  printf(UNDERLINED BOLD "Plugins:" RESET "\n");
  iter(p->plugin, print_registeredplugin);
}



