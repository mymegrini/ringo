#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "protocol/common.h"
#include "protocol/protocol.h"
#include "protocol/network.h"
#include "protocol/thread.h"
#include "shell/shell.h"
#include "plugin_system/plugin_system.h"

#include <pthread.h>
/* #include <readline/readline.h> */
#include "../libreadline/include/readline/readline.h"

#include <getopt.h>



void usage(char *argv0);
void get_info();

#define MODE_CREATE 0
#define MODE_JOIN   1
#define MODE_DUPL   2

#define FLAG_PLUG   1



#define OPT_UDP    'u'
#define OPT_TCP    't'
#define OPT_ID     'i'
#define OPT_JOIN   'j'
#define OPT_DUPL   'd'
#define OPT_PLUG   'p'
#define OPT_VERB   'v'
#define OPT_STRING "u:t:i:jdp::v::"



static struct option options[] = {
    { "udp",    required_argument, 0, OPT_UDP },
    { "tcp",    required_argument, 0, OPT_TCP },
    { "id",     required_argument, 0, OPT_ID },
    { "join",   no_argument, 0, OPT_JOIN },
    { "verbose", optional_argument, 0, OPT_VERB },
    { "plugins", optional_argument, 0, OPT_PLUG },
    { 0,        0, 0, 0 }
};



char *udp_listen = NULL, *tcp_listen = NULL, *id = NULL,
     *host_addr = NULL, *host_port = NULL, *mdiff_ip = NULL, *mdiff_port = NULL;
int mport;
int mode = MODE_CREATE;
int flag = 0;

int main(int argc, char *argv[])
{
    char opt;
    char *plug_dir = plugin_directory;
    int do_free = 0;
    int verbm = VERBM_NOVERB;
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
                mode = MODE_JOIN;
                break;
            case OPT_DUPL:
                mode = MODE_DUPL;
                break;
            case OPT_PLUG:
                if (optarg) {
                  printf("OPT_PLUG-optarg:%s\n", optarg);
                  plug_dir = malloc(strlen(optarg)+1);
                  strcpy(plug_dir, optarg);
                  do_free = 1;
                }
                flag |= FLAG_PLUG;
                break;
            case OPT_VERB:
                if (optarg) {
                  if (strcmp(optarg, "xterm") == 0)
                    verbm = VERBM_XTERMO;
                  else if (strcmp(optarg, "stdout") == 0)
                    verbm = VERBM_STDOUT;
                  else if (strcmp(optarg, "none") == 0)
                    verbm = VERBM_NOVERB;
                  else {
                    fprintf(stderr, "Verbosity option is one of xterm, stdout or none, default is none.\n");
                    printf("optarg:%s\n", optarg);
                    return EXIT_FAILURE;
                  }
                }
                else
                  verbm = VERBM_NOVERB;
                break;
            default:
                usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    get_info();
    init_entity(id, atoi(udp_listen), atoi(tcp_listen), mdiff_ip, mport);
    verbosity(verbm);
    plugin_manager_init(&plugin_manager);
    init_mutex();
    
    switch(mode) {
        case MODE_CREATE:
            if (!create_ring2(mdiff_ip, mport)) {
                fprintf(stderr, "An error occur during ring creation.\n");
                return EXIT_FAILURE;
            }
            break;
        case MODE_JOIN:
            if (!join2(host_addr, host_port)) {
                fprintf(stderr, "An error occur during insertion.\n");
                return EXIT_FAILURE;
            }
            break;
        case MODE_DUPL:
            if (!duplicate_rqst2(host_addr, host_port, mdiff_ip, mport)) {
                fprintf(stderr, "An error occur during duplication\n");
                return EXIT_FAILURE;
            }
            break;
    }

    if ( flag & FLAG_PLUG && ! load_all_plugins(&plugin_manager, plug_dir))
      return EXIT_FAILURE;

    if (do_free)
      free(plug_dir);

    pthread_create(&thread->shell, NULL, run_shell, NULL);
    pthread_join(thread->shell, NULL);

    rl_deprep_terminal();
    printf("\n");
    unload_all_plugins(&plugin_manager);
    printf(RED "\"-_-\"\n");

    exit(0);
}



void usage(char *argv0) {
    printf("Usage:\t%s -u <UDP_PORT> -t <TCP_PORT> -i <ID> [-c HOST PORT] -p\n",
            argv0);
}



void get_info() {
    size_t size = 0;
    if (mode) {
        if (host_addr == NULL) {
            printf("Choose the adress/hostname of entity on the ring you'd like to join: ");
            if (getline(&host_addr, &size, stdin)==-1) perror("getline");
            host_addr[strlen(host_addr)-1] = 0;
            if (strcmp(host_addr, "localhost") == 0) {
                host_addr = realloc(host_addr, 200);
                gethostname(host_addr, 200);
            }
            size = 0;
        }
        if (host_port == NULL) {
            printf("Choose the port used to connect with entity on the ring: ");
            if (getline(&host_port, &size, stdin)==-1) perror("getline");
            host_port[strlen(host_port)-1] = 0;
            size = 0;
        }
    }
    if (udp_listen == NULL) {
        printf("Choose the UDP port you want to use to get messages from the ring (1024-9999): ");
        if (getline(&udp_listen, &size, stdin)==-1) perror("getline");
        udp_listen[strlen(udp_listen)-1] = 0; 
        size = 0;
    }
    if (tcp_listen == NULL) {
        printf("Choose the TCP port you want to use (1024-9999): ");
        if (getline(&tcp_listen, &size, stdin)==-1) perror("getline");
        tcp_listen[strlen(tcp_listen)-1] = 0;
        size = 0;
    }
    if (id == NULL) {
        printf("Choose a nickname (8 chars): ");
        if (getline(&id, &size, stdin)==-1) perror("getline");
        id[strlen(id)-1] = 0;
        size = 0;
    }
    if (mode != MODE_JOIN && mdiff_ip == NULL) {
        printf("Choose a multi diffusion ip (default 225.0.0.1): ");
        if (getline(&mdiff_ip, &size, stdin)==-1) perror("getline");
        mdiff_ip[strlen(mdiff_ip)-1] = 0;
        if (mdiff_ip[0] == 0) {
            mdiff_ip = realloc(mdiff_ip, 16);
            strcpy(mdiff_ip, "225.0.0.1");
        }
        size = 0;
    }
    if (mode != MODE_JOIN && mdiff_port == NULL) {
        printf("Choose a multi diffusion port (default 6666): ");
        if (getline(&mdiff_port, &size, stdin)==-1) perror("getline");
        mdiff_port[strlen(mdiff_port)-1] = 0;
        if (mdiff_port[0] == 0)
            mport = 6666;
        else
            mport = atoi(mdiff_port);
        
    }
    if (mode == MODE_JOIN) {
        mdiff_ip = NULL;
        mport = 0;
    }
}


