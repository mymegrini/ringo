#include <stdio.h>

#include "protocol.h"
#include "network.h"

#include <unistd.h> // temporary here for the sleep

int main(int argc, char *argv[])
{
    if (argc == 1) {
        launch_insserv();
        while (1) {
            printf("\x1b[31mI am waiting for somehing else to do...\x1b[0m\n");
            sleep(10);
        }
    }
    else if (insert(argv[1], argv[2]))
        printf("Well done sir !\n");
    else 
        printf("Oops !\n");
    return 0;
}
