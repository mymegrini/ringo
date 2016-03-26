#include <stdio.h>

#include "protocol.h"
#include "shell.h"

#include <unistd.h> // temporary here for the sleep

int main(int argc, char *argv[])
{
    if (argc == 1) {
        launch_insserv();
        run_shell();
    }
    else if (insert(argv[1], argv[2]))
        run_shell();
    else 
        printf("Oops !\n");
    return 0;
}
