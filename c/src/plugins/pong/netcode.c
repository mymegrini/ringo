#include <stdio.h>
#include <string.h>
#include "../../plugin_system/protocol_interface.h"
#include "pong.h"
#include "engine.h"
#define DEBUG_NETCODE

#define TYPE_SIZE 6
#define LOGIN     "LOGIN "
#define STATE     "STATE "
#define HANDSHAKE "SHAKE "

static char id[ID_SIZE+1] = { 0 };

/**
 * This function parses LOGIN type packets
 */
static int parseLogin(const char* message, const char* content, int lookup_flag){

    #ifdef DEBUG_NETCODE
    printf("%s parsing login attempt by %s\n", id, content);
    #endif
    if(*id != 0 && !engineState()){
	if(strncmp(content, id, ID_SIZE) == 0){
	    #ifdef DEBUG_NETCODE
	    if (!lookup_flag) printf("possible id collision : %s\n", id);
	    #endif
	}
	else {
	    #ifdef DEBUG_NETCODE
	    printf("creating new session\n");
	    #endif
	    createSession(id, content, 0);
	    #ifdef DEBUG_NETCODE
	    printf("sending handshake\n");
	    #endif
	    send_message(PONG_TYPE, "%s%s%s", HANDSHAKE, opponent, self);
	}
    } else if (!lookup_flag){
	    #ifdef DEBUG_NETCODE
	    printf("retransmitting message\n");
	    #endif
	    retransmit(message);
    }
    #ifdef DEBUG_NETCODE
    printf("parsing complete\n");
    #endif
    
    return 0;
}

/**
 * This function parses HANDSHAKE type packets
 */
static int parseShake(const char* message, const char* content, int lookup_flag){

    if(*id != 0 && !engineState() && strncmp(content, id, ID_SIZE) == 0){
        #ifdef DEBUG_NETCODE
	printf("creating session from handshake\n");
        #endif
	createSession(id, content+ID_SIZE, 1);
        #ifdef DEBUG_NETCODE
	printf("acknowledging handshake\n");
        #endif
	send_message(PONG_TYPE, "%s%s%s", HANDSHAKE, opponent, self);
    } else if (!lookup_flag)	
	retransmit(message);
    
    return 0;
}

/**
 * This function parses STATE type packets
 */
static int parseState(const char* message, const char* content, int lookup_flag){

    return 0;
}

/**
 * This function parses pong application messages
 */
int parsePong(const char* message, const char* content, int lookup_flag){

    #ifdef DEBUG_NETCODE
    printf("received : %s\n", message);
    #endif
    if (strncmp(LOGIN, content, TYPE_SIZE) == 0)
	return parseLogin(message, content+TYPE_SIZE, lookup_flag);
    if (strncmp(HANDSHAKE, content, TYPE_SIZE) == 0)
	return parseShake(message, content+TYPE_SIZE, lookup_flag);
    return 1;
}

/**
 * This function tries to establish a connection to another pong application
 */
void loginPong(){

    #ifdef DEBUG_NETCODE
    printf("creating player id\n");
    #endif
    playerid(id);
    #ifdef DEBUG_NETCODE
    printf("attempting to log in\n");
    #endif
    send_message(PONG_TYPE, "%s%s", LOGIN, id);
    #ifdef DEBUG_NETCODE
    printf("sent : %s %s%s\n", PONG_TYPE, LOGIN, id);
    #endif
    return;
}
