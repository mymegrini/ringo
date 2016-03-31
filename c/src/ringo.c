#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "protocol.h"
#include "shell.h"

#include <pthread.h>

#include <getopt.h>



void usage(char *argv0);
void get_info();



#define OPT_UDP    'u'
#define OPT_TCP    't'
#define OPT_ID     'i'
#define OPT_JOIN   'j'
#define OPT_STRING "u:t:i:j"



struct option options[] = {
    { "udp", required_argument, 0, OPT_UDP },
    { "tcp", required_argument, 0, OPT_TCP },
    { "id", required_argument, 0, OPT_ID },
    { "join", no_argument, 0, OPT_JOIN },
    { 0, 0, 0, 0 }
};


char *udp_listen = NULL, *tcp_listen = NULL, *id = NULL,
     *host_addr = NULL, *host_port = NULL;

int flag_insert = 0;


int main(int argc, char *argv[])
{
    char opt;
    while ((opt = getopt_long( argc, argv, OPT_STRING, options, 0 )) != -1   ) {
        switch(opt) {
            case OPT_UDP:
                udp_listen = optarg;
                break;
            case OPT_TCP:
                tcp_listen = optarg;
                break;
            case OPT_ID:
                id = optarg;
                break;
            case OPT_JOIN:
                flag_insert = 1;
                break;
            default:
                usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    get_info();
    init_entity(id, atoi(udp_listen), atoi(tcp_listen));
    
    if (flag_insert) {
        if (!insert(host_addr, host_port)) {
            fprintf(stderr, "An error occur on insertion.\n");
            return EXIT_FAILURE;
        }
    } else {
        create_ring();
    }

    run_shell();

    return EXIT_FAILURE;
}



void usage(char *argv0) {
    printf("Usage:\t%s -u <UDP_PORT> -t <TCP_PORT> -i <ID> [-c HOST PORT]\n",
            argv0);
}



void get_info() {
    size_t size = 0;
    if (flag_insert) {
        if (host_addr == NULL) {
            printf("Choose the adress/hostname of entity on the ring you'd like to join: ");
            getline(&host_addr, &size, stdin);
            host_addr[strlen(host_addr)-1] = 0;
            size = 0;
        }
        if (host_port == NULL) {
            printf("Chosee the port used to connect with entity on the ring: ");
            getline(&host_port, &size, stdin);
            host_port[strlen(host_port)-1] = 0;
            size = 0;
        }
    }
    if (udp_listen == NULL) {
        printf("Chosee the UDP port you want to use to get messages from the ring (1024-9999): ");
        getline(&udp_listen, &size, stdin);
        udp_listen[strlen(udp_listen)-1] = 0; 
        size = 0;
    }
    if (tcp_listen == NULL) {
        printf("Chosee the TCP port you want to use to let other client join the ring from your position (1024-9999): ");
        getline(&tcp_listen, &size, stdin);
        tcp_listen[strlen(tcp_listen)-1] = 0;
        size = 0;
    }
    if (id == NULL) {
        printf("Chosee a nickname (8 chars): ");
        getline(&id, &size, stdin);
        id[strlen(id)-1] = 0;
    }

}

