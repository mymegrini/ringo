#ifndef COMMON_h
#define COMMON_H


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
            printf(BOLD UNDERLINED REVERSE \
            "DEBUG IN " __FILE__ \
            " LINE %d:\n" \
            RESET UNDERLINED REVERSE \
            "In " funcname "\n" \
            RESET REVERSE format "\n" RESET, __LINE__, ##__VA_ARGS__);\
            fflush(stdout)
#else
#define debug(funcname, format, ...)
#endif



#define verbose(format, ...) printf(BOLD "verbose" RESET " - " \
       format, ##__VA_ARGS__)


#define BUFSIZE 100


#define CTNT_OFFST 5

int isnumeric(const char *str);
char *itoa4(int i);
int yesno(const char *question);
int yesnod(const char *question, const int yes);
void printpacket(const char *packet);
int isip(const char *str);
int isport(const char *str);
char *ipnozeros(const char *ip);

#endif /* COMMON_H */
