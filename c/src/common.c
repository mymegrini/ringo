#include "common.h"

#include <ctype.h>
#include <unistd.h>
#include <string.h>

int isnumeric(const char *str) {
    while(*str)
    {
        if(!isdigit(*str))
            return 0;
        str++;
    }

    return 1;
}


/*
 *char *itoa4(int i) {
 *    char *s = (char *)malloc(5);
 *    s[4] = 0;
 *    s[3] = 48 + i % 10;
 *    i /= 10;
 *    s[2] = 48 + i % 10;
 *    i /= 10;
 *    s[1] = 48 + i % 10;
 *    i /= 10;
 *    s[0] = 48 + i;
 *    return s;
 *}
 */
void itoa4(char *s, int i) {
    s[4] = 0;
    s[3] = 48 + i % 10;
    i /= 10;
    s[2] = 48 + i % 10;
    i /= 10;
    s[1] = 48 + i % 10;
    i /= 10;
    s[0] = 48 + i;
}


int yesno(const char *question) {
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


int yesnod(const char *question, const int yes) {
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


void printpacket(const char *packet) {
    printf("---\n%s\n---\n", packet);
}



int isip(const char *str) {
    if (//strlen(str) == 15 && 
            str[3] == '.' && str[7] == '.' && str[11] == '.') {
        return 
            isdigit(str[0]) && isdigit(str[1]) && isdigit(str[2]) &&
            isdigit(str[4]) && isdigit(str[5]) && isdigit(str[6]) &&
            isdigit(str[8]) && isdigit(str[9]) && isdigit(str[10]) &&
            isdigit(str[12]) && isdigit(str[13]) && isdigit(str[14]);
    }
    return 0;
}


int isport(const char *str) {
    return //strlen(str) == 4 &&
        isdigit(str[0]) && isdigit(str[1]) && isdigit(str[2]) &&
        isdigit(str[3]);
}

void ipnozeros(char *nozeros, const char *ip) {
    int j = 0;
    int lz = 0;
    for (int i = 0; i < 16; i++) {
        if ( ! (ip[i] == '0' && lz) ) {
            nozeros[j++] = ip[i];
            if (ip[i] == '.')
                lz = 1;
            else
                lz = 0;
        }
        else if (ip[i+1] == '.') {
            nozeros[j++] = ip[i];
        } 
    }
}
