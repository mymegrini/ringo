#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>


////////////////////////////////////////////////////////////////////////////////
// LOCAL
////////////////////////////////////////////////////////////////////////////////

static char *ipresize(char *ip) {
    char *cpy     = strdup(ip);

    //ip            = (char *)realloc((void*)ip, 16);
    char *token   = strtok(cpy, ".");
    int  len      = strlen(token);
    int i = 0;
    for (i = 0; i < 3-len; i++)
        ip[i] = '0';
    for (int j = 0; j < len; j++)
        ip[i+j] = token[j];
    for(int field = 1; field < 4; field++) {
        ip[4*field-1] = '.';
        token = strtok(NULL, ".");
        len   = strlen(token);
        for (i = 0; i < 3-len; i++)
            ip[i+4*field] = '0';
        for (int j = 0; j < 3; j++)
            ip[i+j+4*field] = token[j];
    }
    ip[15] = 0;
    free(cpy);
    return ip;
}


////////////////////////////////////////////////////////////////////////////////
// GLOBAL
////////////////////////////////////////////////////////////////////////////////

char *getIp(const char *hostname) {
    struct hostent *he;
    struct in_addr **addr_list;
    if ( (he = gethostbyname( hostname ) ) == NULL) {
        debug("getIp(\"%s\")", "gethostbyname(\"%s\") returned NULL\n",
                hostname, hostname);
        return NULL;
    }

    addr_list = (struct in_addr **) he->h_addr_list;

    for(int i = 0; addr_list[i] != NULL; i++) 
    {
        char *ip = (char *) malloc(16);
        // Take the first one
        strcpy(ip, inet_ntoa(*addr_list[i]));
        // Adjust size
        ipresize(ip);
        return ip;
    }
    debug("getIp(\"%s\")", "no addr find in the list.\n", hostname);
    return NULL;
}

char *getLocalIp() {
    char hostname[255];
    gethostname(hostname, 255);
    return getIp(hostname);
}


#define BUFSIZE 100
char *receptLine(const int sock) {
    char *s = malloc(BUFSIZE);
    int size = BUFSIZE;
    int lus = recv(sock, s, BUFSIZE, 0);
    while (s[lus-1] != '\n') {
        if (lus + BUFSIZE > size) {
            size *= 2;
            s = realloc(s, size);
        }
        lus += recv(sock, &s[lus], BUFSIZE, 0);
    }
    s[lus-1] = 0;
    return s;
}
