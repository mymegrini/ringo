#include "common.h"

#include <ctype.h>

int isnumeric(char *str) {
  while(*str)
  {
      if(!isdigit(*str))
        return 0;
      str++;
    }

  return 1;
}

char *itoa4(int i) {
    char *s = (char *)malloc(5);
    s[4] = 0;
    s[3] = 48 + i % 10;
    i /= 10;
    s[2] = 48 + i % 10;
    i /= 10;
    s[1] = 48 + i % 10;
    i /= 10;
    s[0] = 48 + i;
    return s;
}

