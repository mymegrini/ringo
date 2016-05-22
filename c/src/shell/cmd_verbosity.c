#include "../plugin_system/plugin_system.h"
#include "../common.h"

#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>






static void usage(char *argv0);
static void help(char *argv0);

#define   OPT_HELP      'h'
#define   OPTL_HELP     "help"

#define   OPT_STRING    "h"



static struct option longopts[] = {
  {OPTL_HELP,   no_argument,         0,   OPT_HELP},
  {0,           0,                   0,   0}
};



int cmd_verbosity(int argc, char **argv)
{
  if (argc == 1) {
    toggle_verbose();
    return 0;
  }

  int c, indexptr;
  optind = 0;

  while ((c = getopt_long(argc, argv, OPT_STRING,
          longopts, &indexptr)) != -1) {
    switch (c) {
      case OPT_HELP:
        help(argv[0]);
        return 0;
      default:
        usage(argv[0]);
        return 1;
    }
  }

  if (strcmp(argv[optind], "none") == 0)
    verbosity(VERBM_NOVERB);
  else if (strcmp(argv[optind], "stdout") == 0)
    verbosity(VERBM_STDOUT);
  else if (strcmp(argv[optind], "xterm") == 0)
    verbosity(VERBM_XTERMO);
  else {
    fprintf(stderr, "Verbosity can be set to one of none, stdout and xterm.\n");
    return 1;
  }

  return 0;
}



static void usage(char *argv0)
{
  printf("Usage:\t%s [-h] <verbosity_mode>\n", argv0);
}



static void help(char *argv0)
{
  usage(argv0);
  printf("Verbosity mode is one of none, stdout and xterm.\n"
      "No options toggle verbose mode with default mode sets to stdout.\n");
}



