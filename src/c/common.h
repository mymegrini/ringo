#ifndef COMMON_h
#define COMMON_H


#include <stdlib.h>

#define debug(funcname, format, ...) \
    printf("\x1b[1m\x1b[4m\x1b[7mDEBUG IN " __FILE__ " LINE " __LINE__ ":\x1b[0m\n" \
            "\x1b[4m\x1b[7mIn " funcname "\x1b[0m\n"\
             "\x1b[7m" format "\x1b[0m", ##__VA_ARGS__)

#define verbose(format, ...) printf("\x1b[1mverbose\x1b[0m - " \
       format, ##__VA_ARGS__)

int isnumeric(char *str);
char *itoa4(int i);

#endif /* COMMON_H */
