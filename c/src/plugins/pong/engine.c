#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "../../plugin_system/protocol_interface.h"
#include "engine.h"
#include "pong.h"

typedef struct {
    int x;
    int y;
    int vx;
    int vy;
} ball;

typedef struct {
    char id[ID_SIZE+1];
    int racket;
    int score;
} player;
    
typedef struct {
    int w;
    int h;
    player player[2];
    ball ball;
} core;

core* engine = NULL;
int* racket = NULL;

/**
 * Generate a unique player identificator
 *
 * @param content message content to be included in hash
 * @param hash will contain message hash id
 * @return message idenfitificator, a char* of strlen 8
 */
void playerid(char* hash){

    struct timeval time;
    uint64_t h = 5381;
    int i;
    uint8_t c;
    
    /* hash * 33 + c */
    // hashing time
    gettimeofday(&time, NULL);
    h = h * 33 + (uint16_t)time.tv_sec;
    h = h * 33 + (uint16_t)time.tv_usec;
    // hashing ip and port
    for(i=0; i<16; i++) h = h * 33 + info->ip_self[i];  
    h = h * 33 + info->udp;

    // creating hash using alphanumerical characters
    for(i=0; i<ID_SIZE; i++){
        c = h % 62;
        if (c<10) hash[i] = c+48;      //digits
        else if (c<36) hash[i] = c-10+97; //lowercase letters
        else if (c<62) hash[i] = c-36+65; //uppercase letters
        else hash[i] = 0;
    
        h = h / 62;
    }
    hash[ID_SIZE] = 0;
    return;
}

/**
 * This function initializes the engine
 * @param host host id
 * @param guest guest id
 * @param self equals 0 if player is host and 1 if guest
 * @return void
 */
void createSession(const char* host, const char* guest, int self_index){

    //initializing the engine
    engine = malloc(sizeof(core));
    engine->w = FIELD_X;
    engine->h = FIELD_Y;
    strncpy(engine->player[0].id, host, ID_SIZE);
    engine->player[0].id[ID_SIZE] = 0;
    strncpy(engine->player[1].id, guest, ID_SIZE);
    engine->player[1].id[ID_SIZE] = 0;
    engine->player[0].score = 0;
    engine->player[1].score = 0;
    engine->player[0].racket = -1;
    engine->player[1].racket = -1;
    engine->ball.x = ( engine->w - BALL_SIZE )/ 2;
    engine->ball.y = ( engine->h - BALL_SIZE )/ 2;

    //seting up the pointers
    self = engine->player[self_index].id;
    opponent = engine->player[1-self_index].id;
    racket = &(engine->player[self_index].racket);
    *racket = ( engine->h - RACKET_Y )/ 2;

    return;
}

/**
 * This function free engine ressources
 */
void
destroySession(){
    
    free(engine);
    engine = NULL;
    return;
}

/**
 * This function checks if a session is already running
 */
int engineState(){
    if (engine)
	return 1;
    else
	return 0;
}

/**
 * This function returns the state of the game for rendering
 */
void getState(state* s){

    if(engineState()){
	s->available = 1;
	s->score1    = engine->player[0].score;
	s->score2    = engine->player[1].score;	
	s->racket1   = engine->player[0].racket;
	s->racket2   = engine->player[1].racket;
	s->ballx     = engine->ball.x;
	s->bally     = engine->ball.y;
    } else {
	s->available = 0;
    }
    return;
}

/**
 * This function updates the state of the game
 */
void moveRacket(int step){
    *racket += step;
    return;
}
