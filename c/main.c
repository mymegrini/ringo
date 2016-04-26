#include "test.h"
#include <stdio.h>

int main(int argc, char const* argv[])
{
  
  struct r * const cast = (struct r * const)copy;
  printf("%d\n", cast->f);
  return 0;
}
