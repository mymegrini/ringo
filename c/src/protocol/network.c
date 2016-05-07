#include "network.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>


////////////////////////////////////////////////////////////////////////////////
// LOCAL
////////////////////////////////////////////////////////////////////////////////

char *ipresize(const char *ip) {
    int len = strlen(ip);
    char *ipr = malloc(16);
    int j = 14;
    int width = 0;
    for (int i = len-1; i >= 0 ; --i) {
        if (ip[i] == '.') {
            for ( ; width < 3; ++width)
                ipr[j--] = '0';
            ipr[j--] = '.';
            width = 0;
        }
        else {
            ipr[j--] = ip[i];
            ++width;
        }
    }
    for ( ; width < 3; ++width)
        ipr[j--] = '0';
    ipr[15] = 0;

    return ipr;
}

void ipresize_noalloc(char ipr[16], const char *ip) {
    int len = strlen(ip);
    int j = 14;
    int width = 0;
    for (int i = len-1; i >= 0 ; --i) {
        if (ip[i] == '.') {
            for ( ; width < 3; ++width)
                ipr[j--] = '0';
            ipr[j--] = '.';
            width = 0;
        }
        else {
            ipr[j--] = ip[i];
            ++width;
        }
    }
    for ( ; width < 3; ++width)
        ipr[j--] = '0';
    ipr[15] = 0;
}


char *ipresize2(const char *ip2) {
    char *cpy     = strdup(ip2);

    char *ip      = (char *)malloc(16);
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
        char *ipr = ipresize(ip);
        free(ip);
        return ipr;
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


int multicast_subscribe(int sock, int port, const char *ip) {
    int ok = 1;
    int r = setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &ok, sizeof(ok));
    if (r == -1) {
        fprintf(stderr, "Multicast on same machine not working.\n");
        exit(1);
    }
    struct sockaddr_in addr_sock;
    addr_sock.sin_family = AF_INET;
    addr_sock.sin_port = htons(port);
    addr_sock.sin_addr.s_addr = htonl(INADDR_ANY);
    r = bind(sock, (struct sockaddr *) &addr_sock, 
            sizeof(struct sockaddr_in));
    if (r == -1) {
        fprintf(stderr, "Binding error with multicast socket.\n");
        return 0;
    }
    struct ip_mreq mreq;
    if (! inet_aton(ip, &mreq.imr_multiaddr)) {
        fprintf(stderr, "Multicast ip \"%s\" not valid.\n", ip);
        return 0;
    }
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    r = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    if (r == -1) {
        fprintf(stderr, "Can't subscribe to multicast on \"%s\"\n", ip);
        return 0;
    }
    return 1;
}


int bind_udplisten(int sock, int port) {
    struct sockaddr_in address_sock;
    address_sock.sin_family = AF_INET;
    address_sock.sin_port = htons(port);
    address_sock.sin_addr.s_addr = htonl(INADDR_ANY);
    int r=bind(sock, (struct sockaddr *)&address_sock,
            sizeof(struct sockaddr_in));
    if (r == -1) {
        close(sock);
        return 0;
    }
    return sock;
}

int getsockaddr_in(struct sockaddr_in *addr, const char *host, int port, int is_ip) {
    struct in_addr ip;
    struct addrinfo *first_info;
    struct addrinfo hints;
    if (is_ip) {
        if (inet_aton(host, &ip)) {
            addr->sin_family = AF_INET;
            addr->sin_port = htons(port);
            addr->sin_addr = ip;
            return 1;
        } else {
        return 0;
        }
    }
    else {
        if (inet_aton(host, &ip)) {
            addr->sin_family = AF_INET;
            addr->sin_port = htons(port);
            addr->sin_addr = ip;
            return 1;
        } else {
            bzero(&hints, sizeof(struct addrinfo));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            char portstr[5];
            itoa4(portstr, port);
            if (getaddrinfo(host, portstr, &hints, &first_info) == 0 &&
                    first_info != NULL) {
                *addr = *((struct sockaddr_in *) first_info->ai_addr);
                freeaddrinfo(first_info);
                return 1;
            } else {
                return 0;
            }
        }
    }
    return 0;
}
