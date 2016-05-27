#include <stdio.h>
#include <string.h>
#include "../../plugin_system/plugin_programmer_interface.h"
#include "pong.h"
#include "engine.h"

//#define DEBUG_NETCODE

#define TYPE_SIZE 6
#define LOGIN     "LOGIN "
#define STATE     "STATE "
#define HANDSHAKE "SHAKE "
#define LOGOUT    "CLOSE "

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
	    double time = 0;
	    createSession(id, content, 0, &time);
	    #ifdef DEBUG_NETCODE
	    printf("sending handshake\n");
	    #endif
	    send_message(PONG_TYPE, "%s%s%s %lf", HANDSHAKE,
			 opponent, self, time);
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
	char hostId[ID_SIZE+1];
	strncmp(hostId, content+ID_SIZE, 8);
	hostId[ID_SIZE] = 0;
	double time = atof(content+ 2 * ID_SIZE + 1);
	createSession(content+ID_SIZE, id, 1, &time);
        #ifdef DEBUG_NETCODE
	printf("acknowledging handshake %s%s%s\n", HANDSHAKE, opponent, self);
        #endif
	send_message(PONG_TYPE, "%s%s%s", HANDSHAKE, opponent, self);
    } else if (*id != 0 && engineState() &&
	       strncmp(content, self, ID_SIZE) == 0
	       && strncmp(content + ID_SIZE, opponent, ID_SIZE) == 0){
            #ifdef DEBUG_NETCODE
	    printf("handshake acknowledged\n");
            #endif
    } else if (!lookup_flag)
	retransmit(message);

    return 0;
}

/**
 * This function parses LOGOUT type packets
 */
static int parseLogout(const char* message, const char* content, int lookup_flag){

    #ifdef DEBUG_NETCODE
    printf("%s parsing logout message %s\n", id, content);
    #endif
    if(engineState()
       && strncmp(content, self, ID_SIZE) == 0
       && strncmp(content+ID_SIZE, opponent, ID_SIZE) == 0){
        #ifdef DEBUG_NETCODE
	printf("received logout packet\n");
        #endif
	destroySession();
    } else if (!lookup_flag)
	retransmit(message);

    return 0;
}

/**
 * This function parses STATE type packets
 */
static int parseState(const char* message, const char* content, int lookup_flag){

    if (!strncmp(content, opponent, ID_SIZE)
	&& !strncmp(content + ID_SIZE, self, ID_SIZE)
	&& !lookup_flag){
	update s;
	int r = sscanf(content + 2 * ID_SIZE, "%d %d %lf %d %d %d %lf %lf",
		       &s.score, &s.racket, &s.racket_t, &s.ball_x, &s.ball_y,
		       &s.ball_v, &s.ball_d, &s.ball_t);

	if (r == 8){
            #ifdef DEBUG_NETCODE
	    printf("received : S%d R%d T%lf X%d Y%d V%d D%lf T%lf\n",
		   s.score, s.racket, s.racket_t, s.ball_x, s.ball_y,
		   s.ball_v, s.ball_d, s.ball_t);
            #endif
	    updateState(&s);
	} else {
	    return 1;
	}
    } else if(!lookup_flag){
        #ifdef DEBUG_NETCODE
	printf("transmitting status update %s\n", content);
        #endif
	retransmit(message);
    }
    return 0;
}

/**
 * This function parses pong application messages
 */
int parsePong(const char* message, const char* content, int lookup_flag){

    if (strncmp(STATE, content, TYPE_SIZE) == 0)
	return parseState(message, content+TYPE_SIZE, lookup_flag);
    if (strncmp(LOGIN, content, TYPE_SIZE) == 0)
	return parseLogin(message, content+TYPE_SIZE, lookup_flag);
    if (strncmp(HANDSHAKE, content, TYPE_SIZE) == 0)
	return parseShake(message, content+TYPE_SIZE, lookup_flag);
    if (strncmp(LOGOUT, content, TYPE_SIZE) == 0)
	return parseLogout(message, content+TYPE_SIZE, lookup_flag);
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
    printf("sent     : %s %s%s\n", PONG_TYPE, LOGIN, id);
    #endif
    return;
}

/**
 * This function sends updates
 */
#define RACKET_RES 5
#define CMP_UPDATE(new, old) (abs(new.racket - old.racket) > RACKET_RES || \
			      new.ball_x != old.ball_x ||		\
			      new.ball_y != old.ball_y ||		\
			      new.ball_d != old.ball_d ||		\
			      new.ball_v != old.ball_v)

void sendUpdate(){

    static update old_state;
    update s;
    getUpdate(&s);
    if (s.state && CMP_UPDATE(s, old_state)){
	old_state = s;
        #ifdef DEBUG_NETCODE
	printf("Sending an update\n");
        #endif
	send_message(PONG_TYPE, "%s%s%s %d %d %lf %d %d %d %lf %lf", STATE,
		     self, opponent, s.score, s.racket, s.racket_t,
		     s.ball_x, s.ball_y, s.ball_v, s.ball_d, s.ball_t);
        #ifdef DEBUG_NETCODE
	printf("sent     : %s%s%s S%d R%d T%lf X%d Y%d V%d D%lf T%lf\n",
	       STATE, self, opponent, s.score, s.racket, s.racket_t,
	       s.ball_x, s.ball_y, s.ball_v, s.ball_d, s.ball_t);
        #endif
    }

    return;
}

/**
 * This function takes care of closing a session
 */
void logoutPong(){

    if(engineState()){

        #ifdef DEBUG_NETCODE
	printf("sending logout message\n");
        #endif
	send_message(PONG_TYPE, "%s%s%s", LOGOUT, opponent, self);
        #ifdef DEBUG_NETCODE
	printf("sent     : %s %s%s%s\n", PONG_TYPE, LOGOUT, opponent, self);
        #endif

        #ifdef DEBUG_NETCODE
	printf("closing engine\n");
        #endif
	destroySession();
    }

    #ifdef DEBUG_NETCODE
    printf("deleting player id\n");
    #endif
    *id = 0;
    return;
}
