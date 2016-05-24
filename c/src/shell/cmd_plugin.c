#include "../plugin_system/plugin_system.h"
#include "../common.h"

#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>






static void usage(char *argv0);
static void help(char *argv0);
static void list_available(char *plug_dir);

#define   OPT_HELP      'h'
#define   OPTL_HELP     "help"
#define   OPT_LOAD      'o'
#define   OPTL_LOAD     "open"
#define   OPT_UNLOAD    'c'
#define   OPTL_UNLOAD   "close"
#define   OPT_LIST      'l'
#define   OPTL_LIST     "list"

#define   OPT_STRING    "ho:c:l::"



static struct option longopts[] = {
  {OPTL_HELP,   no_argument,         0,   OPT_HELP},
  {OPTL_LOAD,   required_argument,   0,   OPT_LOAD},
  {OPTL_UNLOAD, required_argument,   0,   OPT_UNLOAD},
  {OPTL_LIST,   optional_argument,   0,   OPT_LIST},
  {0,           0,                   0,   0}
};





#define   FLAG_LOAD     1
#define   FLAG_UNLOAD   2
#define   FLAG_LIST     4



int cmd_plugin(int argc, char **argv)
{
  char *loadplug, *unloadplug;
  int c, indexptr;
  short flag = 0;
  optind = 0;
  char *list_path = plugin_directory;
  int do_free = 0;

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
      case OPT_LIST:
        if (optarg) {
          list_path = malloc(strlen(optarg)+1);
          strcpy(list_path, optarg);
          do_free = 1;
        }
        flag |= FLAG_LIST;
        break;
      default:
        usage(argv[0]);
        return 1;
        break;
    }
  }
  if (flag == 0) {
    show_plugins();
    return 0;
  }

  if (flag & FLAG_LIST) 
    list_available(list_path);

  if (do_free)
    free(list_path);

  if (flag & FLAG_UNLOAD) {
    if (!unloadplugin(&plugin_manager, unloadplug))
      return 1;
  }
  if (flag & FLAG_LOAD) {
    if (!loadplugin(&plugin_manager, plugin_directory, loadplug))
      return 1;
  }

  return 0;
}



static void usage(char *argv0)
{
  printf("Usage:\t%s [-o plugins] [-c plugins] [-h] [-l <directory>]\n", argv0);
}



static void help(char *argv0)
{
  usage(argv0);
}



#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

static void list_available(char *plug_dir)
{
  DIR *dir;
  if ((dir = opendir (plug_dir)) != NULL) {
    /* print all the files and directories within directory */
    struct dirent *ent;
    char wd[256];
    if(getcwd(wd, 256) == NULL){
	perror("list_available");
	return ;
    }
    if(chdir(plug_dir)){
	perror("list_available");
	return ;
    }
    while ((ent = readdir (dir)) != NULL) {
      struct stat info;
      /* debug("load_all_plugins", "file: %s, S_ISREG: %d, S_ISDIR: %d", ent->d_name, S_ISREG(info.st_mode), S_ISDIR(info.st_mode)); */
      if (! S_ISDIR(info.st_mode)) {
        char *plugname;
        if (plugin_extract_name(&plugname, ent->d_name)) {
          printf("\t- " BOLD "%s\n" RESET, plugname);
          free(plugname);
        }
      }
    }
    if(chdir(wd)){
	perror("list_available");
	return ;
    }
    closedir(dir);
  }
  else {
    fprintf(stderr, "Can't open directory %s.\n", plug_dir);
  }
}
