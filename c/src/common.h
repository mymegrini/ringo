#ifndef COMMON_h
#define COMMON_H


#include <stdlib.h>
#include <stdio.h>

// TERMINAL COLORS AND FORMATTING
#define RESET         "\x1b[0m"

#define BOLD          "\x1b[1m"
#define DIM           "\x1b[2m"
#define UNDERLINED    "\x1b[4m"
#define BLINK         "\x1b[5m"
#define REVERSE       "\x1b[7m"
#define HIDDEN        "\x1b[8m"

#define RED           "\x1b[31m"
#define GREEN         "\x1b[32m"
#define YELLOW        "\x1b[33m"
#define BLUE          "\x1b[34m"
#define MAGENTA       "\x1b[35m"
#define CYAN          "\x1b[36m"


#ifdef DEBUG
#define debug(funcname, format, ...) \
    printf("\x1b[1m\x1b[4m\x1b[7mDEBUG IN " __FILE__ " LINE " __LINE__ ":\x1b[0m\n" \
            "\x1b[4m\x1b[7mIn " funcname "\x1b[0m\n"\
             "\x1b[7m" format "\x1b[0m", ##__VA_ARGS__)
#else
#define debug(funcname, format, ...)
#endif


#define verbose(format, ...) printf("\x1b[1mverbose\x1b[0m - " \
       format, ##__VA_ARGS__)


#define BUFSIZE 100


int isnumeric(char *str);
char *itoa4(int i);
int yesno(char *question);
int yesnod(char *question, int yes);

#endif /* COMMON_H */
