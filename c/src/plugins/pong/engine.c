#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

#define DEBUG_ENGINE

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
    int self;
    int w;
    int h;
    player player[2];
    ball ball;
} core;

SDL_mutex* mutex = NULL;
core* engine = NULL;
int* racket = NULL;

/**
 * This function returns the current time
 */
double clock(){

    struct timeval time;
    gettimeofday(&time, NULL);
    return (double) time.tv_sec + time.tv_usec / 1000;
}

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
 * This function initializes the mutex
 */
void initEngine(){

    if (mutex == NULL) {

        #ifdef DEBUG_ENGINE
	printf("creating mutex\n");
        #endif

	mutex = SDL_CreateMutex();
	if (!mutex)
	    printf("initEngine : Couldn't create mutex\n");
    }

    return;
}

/**
 * This function destroys the mutex
 */
void quitEngine(){

    if(mutex != NULL){

        #ifdef DEBUG_ENGINE
        printf("destroying mutex\n");
        #endif

	SDL_DestroyMutex(mutex);
	mutex = NULL;
    }
    return;
}

/**
 * This function locks the engine
 */
void lockEngine(){

    if (mutex && SDL_LockMutex(mutex))
	printf("%s: %s\n", __func__, SDL_GetError());

    return;
}

/**
 * This function unlocks the engine
 */
void unlockEngine(){

    if (mutex && SDL_UnlockMutex(mutex))
	printf("%s: %s\n", __func__, SDL_GetError());

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

    if (SDL_LockMutex(mutex))
	printf("%s: %s\n", __func__, SDL_GetError());
    else {

	//initializing the engine
	engine = malloc(sizeof(core));
	engine->self = self_index;
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
	self = engine->player[engine->self].id;
	opponent = engine->player[1-engine->self].id;
	engine->player[engine->self].racket = ( engine->h - BALL_SIZE )/ 2;

	//Freeing mutex
	SDL_UnlockMutex(mutex);
    }

    return;
}

/**
 * This function free engine ressources
 */
void
destroySession(){

    if (SDL_LockMutex(mutex))
	printf("%s: %s\n", __func__, SDL_GetError());
    else {

	free(engine);
	engine = NULL;
	self = NULL;
	opponent = NULL;

	//Freeing mutex
	SDL_UnlockMutex(mutex);
    }
    return;
}

/**
 * This function checks if a session is already running
 */
int engineState(){

    int r = 0;

    if(mutex){
	if (SDL_LockMutex(mutex))
	    printf("%s: %s\n", __func__, SDL_GetError());
	else {
	    if (engine)
		r = 1;

	    //Freeing mutex
	    SDL_UnlockMutex(mutex);
	}
    }

    return r;
}

/**
 * This function returns the state of the game for rendering
 */
void getState(state* s){

    if (SDL_LockMutex(mutex))
	printf("%s: %s\n", __func__, SDL_GetError());
    else {

	if(engineState()){
	    s->available = 1;
	    s->score[0]    = engine->player[0].score;
	    s->score[1]    = engine->player[1].score;
	    s->racket[0]   = engine->player[0].racket;
	    s->racket[1]   = engine->player[1].racket;
	    s->ball[0]     = engine->ball.x;
	    s->ball[1]     = engine->ball.y;
	} else {
	    s->available = 0;
	}

	//Freeing mutex
	SDL_UnlockMutex(mutex);
    }
    return;
}

/**
 * This function updates the state of the game
 * returns 1 if modified, 0 if not
 */
int moveRacket(int step){

    int update = 1;

    if (SDL_LockMutex(mutex)){
	printf("%s: %s\n", __func__, SDL_GetError());
	return 0;
    } else {

	int* r = &engine->player[engine->self].racket;

	if(*r + step < 0)
	    if(*r != 0)
		*r = 0;
	    else
		update = 0;


	else if(*r + step > engine->h - RACKET_Y)
	    if(*r != engine->h - RACKET_Y)
		*r = engine->h - RACKET_Y;
	    else
		update = 0;

	else
	    *r += step;

	//Freeing mutex
	SDL_UnlockMutex(mutex);

	return update;
    }

}

/**
 * This function updates state
 */
void updateState(const state* s){

    if (SDL_LockMutex(mutex))
	printf("%s: %s\n", __func__, SDL_GetError());
    else {

	int opponent = 1 - engine->self;
	engine->player[opponent].racket = s->racket[opponent];
	//engine->score[opponent] = s.score[opponent];

	//Freeing mutex
	SDL_UnlockMutex(mutex);
    }
    return;
}
