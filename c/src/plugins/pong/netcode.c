#include <stdio.h>
#include "../../plugin_system/plugin_interface.h"
#include "physics.h"

/**
 * This function parses pong application messages
 */
int parsePong(const char* message, const char* content, int lookup_flag){

    printf("%ld\n", content-message);
    return 0;
}

/**
 * This function tries to establish a connection to another pong application
 */
int
login(){

    return 0;
}
