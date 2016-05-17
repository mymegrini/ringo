#include <stdio.h>
#include "../banner.h"
#include "../common.h"



void print_team()
{
  printf("%s%s\n"
      "%s%s\n"
      "%s%s\n",
      nicolas[0], nicolas[1], soheib[0], soheib[1], younes[0], younes[1]);
}

int cmd_info(int argc, char *argv[])
{
  print_team();
  return 0;
}
