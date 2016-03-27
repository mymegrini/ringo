#include <stdio.h>

#include "protocol.h"
#include "shell.h"

#include <pthread.h>

#include <getopt.h>

#define OPT_UDP    'u'
#define OPT_TCP    't'
#define OPT_INSERT 'i'
#define OPT_STRING "u:t:c"

int main(int argc, char *argv[])
{
    // FIRST there should be a call to 
    // void init_entity(char *id, uint16_t udp_listen, uint16_t tcp_listen)

    if (argc == 1) {
        init_entity("Bryan", 4242, 4343);
        create_ring();
        run_shell();
    }
    else {
        init_entity("Peter", 5252, 5353);
        if (insert(argv[1], argv[2])) {
            pthread_t t_message_manager;
            pthread_create(&t_message_manager, NULL, message_manager, NULL);
            run_shell();
        }
        printf("Oops !\n");
    }
    return 0;
}
