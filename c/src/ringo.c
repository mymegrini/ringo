#include <stdio.h>

#include "protocol.h"
#include "shell.h"

#include <pthread.h>

int main(int argc, char *argv[])
{
    // FIRST there should be a call to 
    // void init_entity(char *id, uint16_t udp_listen, uint16_t tcp_listen)
    init_entity("Bryan", 4242, 4343);

    if (argc == 1) {
        create_ring();
        pthread_t t_insertion_server;
        pthread_create(&t_insertion_server, NULL, insertion_server, NULL);
        pthread_t t_message_manager;
        pthread_create(&t_message_manager, NULL, message_manager, NULL);
        run_shell();
    }
    else if (insert(argv[1], argv[2]))
        run_shell();
    else 
        printf("Oops !\n");
    return 0;
}
