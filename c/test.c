#include "src/protocol/common.h"

#include <stdio.h>

#include <signal.h>


int main(int argc, char const* argv[])
{
  sigset_t mask;
  sigfillset(&mask);
  sigprocmask(SIG_SETMASK, &mask, NULL);
  char buff[100];
  int fd = init_outputxterm();
  while (1) {
    printf(">");
    scanf("%s", buff);
    dprintf(fd, "%s\n", buff);
  }
  return 0;
}
