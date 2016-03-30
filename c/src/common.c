#include "common.h"

#include <ctype.h>
#include <unistd.h>
#include <string.h>

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


int yesno(char *question) {
    char *ans = NULL;
    int yes;
    do {
        printf("%s " BOLD "[y/n] " RESET, question);
        size_t lus = 0;
        lus = getline(&ans, &lus, stdin); 
        ans[lus-1] = 0;
        *ans = toupper(*ans);
        yes = strcmp(ans, "Y");
    } while(yes != 0 && strcmp(ans, "N") != 0);
    free(ans);
    return yes == 0;
}


int yesnod(char *question, int yes) {
    char *ans = NULL;
    printf("%s " BOLD "[y/n] " RESET, question);
    size_t lus = 0;
    lus = getline(&ans, &lus, stdin); 
    ans[lus-1] = 0;
    *ans = toupper(*ans);
    int response;
    if (yes)
        response = strcmp(ans, "N") != 0;
    else
        response = strcmp(ans, "Y") == 0;
    free(ans);
    return response;
}


void printpacket(char *packet) {
    printf("---\n%s\n---\n", packet);
}





