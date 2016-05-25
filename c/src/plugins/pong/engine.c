#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

#define DEBUG_ENGINE

#include "../../plugin_system/plugin_programmer_interface.h"
#include "engine.h"
#include "pong.h"

typedef struct {
    double t;
    double x;
    double y;
    double v;
    double d;
} ball;

typedef struct {
    char id[ID_SIZE+1];
    double time;
    double racket;
    int score;
} player;

typedef struct {
    double time;
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
double tick(){

    struct timespec time;

    if (clock_gettime(CLOCK_REALTIME_COARSE, &time) == -1){
	perror("clock");
	return -1;
    }

    //printf("%ld,%ld\n", time.tv_sec, time.tv_nsec);
    int msec = time.tv_nsec / 100000;
    return (double)time.tv_sec + msec/1000.0;
}

/**
 * Generate a unique player identificator
 *
 * @param content message content to be included in hash
 * @param hash will contain message hash id
 * @return message idenfitificator, a char* of strlen 8
 */
void playerid(char* hash){

    struct timespec time;
    uint64_t h = 5381;
    int i;
    uint8_t c;

    /* hash * 33 + c */
    // hashing time
    if (clock_gettime(CLOCK_REALTIME_COARSE, &time) == -1){
	perror("clock");
	return;
    }
    h = h * 33 + (uint16_t)time.tv_sec;
    h = h * 33 + (uint16_t)time.tv_nsec;
    // hashing ip and port
    /* for(i=0; i<16; i++) h = h * 33 + info->ip_self[i]; */
    /* h = h * 33 + info->udp; */
    char ip[16];
    for(i=0; i<16; i++) h = h * 33 + ip[i];
    h = h * 33 + get_udp();

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
void createSession(const char* host, const char* guest,
		   int self_index, double* time){

    if (SDL_LockMutex(mutex))
	printf("%s: %s\n", __func__, SDL_GetError());
    else {

	//initializing the engine
	engine = malloc(sizeof(core));
	engine->time = tick();
	engine->self = self_index;
	engine->w = FIELD_W;
	engine->h = FIELD_H;
	//initializing players
	strncpy(engine->player[0].id, host, ID_SIZE);
	engine->player[0].id[ID_SIZE] = 0;
	strncpy(engine->player[1].id, guest, ID_SIZE);
	engine->player[1].id[ID_SIZE] = 0;
	engine->player[0].score = 0;
	engine->player[1].score = 0;
	engine->player[0].racket = -1;
	engine->player[1].racket = -1;
	//initializing ball
	if(*time == 0){
	    engine->ball.t = engine->time + 3;
	    *time = engine->ball.t;
	} else {
	    engine->ball.t = *time;
	}
	engine->ball.x = engine->w / 2;
	engine->ball.y = engine->h / 2;
	engine->ball.v = BALL_V / 2;
	engine->ball.d = M_PI;

	//seting up the pointers
	self = engine->player[engine->self].id;
	opponent = engine->player[1-engine->self].id;
	engine->player[engine->self].racket = engine->h / 2;

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
	    if (engine != NULL)
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
static void moveBall(double* b, double time){

    ball* ball = &(engine->ball);
    b[0] = ball->x + ball->v * cos(ball->d) * (time - ball->t);
    b[1] = ball->y + ball->v * sin(ball->d) * (time - ball->t);
}
void getState(state* s){

    if (SDL_LockMutex(mutex))
	printf("%s: %s\n", __func__, SDL_GetError());
    else {

	if(engine != NULL){
	    s->available = 1;
	    s->score[0]    = engine->player[0].score;
	    s->score[1]    = engine->player[1].score;
	    s->racket[0]   = (int) engine->player[0].racket;
	    s->racket[1]   = (int) engine->player[1].racket;
	    //obtain ball coordinates
	    if(engine->time > engine->ball.t){
		double b[2];
		moveBall(b, engine->time);
		s->ball[0] = (int) b[0];
		s->ball[1] = (int) b[1];
	    } else {
		s->ball[0] = engine->ball.x;
		s->ball[1] = engine->ball.y;
	    }
	} else {
	    s->available = 0;
	}

	//Freeing mutex
	SDL_UnlockMutex(mutex);
    }
    return;
}

void getUpdate(update* u){

    if (SDL_LockMutex(mutex))
	printf("%s: %s\n", __func__, SDL_GetError());
    else {

	if(engine != NULL){
	    u->score    = engine->player[1-engine->self].score;
	    u->racket   = engine->player[engine->self].racket;
	    u->racket_t = engine->player[engine->self].time;
	    u->ball_x   = engine->ball.x;
	    u->ball_y   = engine->ball.y;
	    u->ball_v   = engine->ball.v;
	    u->ball_d   = engine->ball.d;
	    u->ball_t   = engine->ball.t;
	} else {
	    u = NULL;
	}

	//Freeing mutex
	SDL_UnlockMutex(mutex);
    }
    return;
}

/**
 * This function updates state
 */
void updateState(const update* u){

    if (SDL_LockMutex(mutex))
	printf("%s: %s\n", __func__, SDL_GetError());
    else {

	int opponent = 1 - engine->self;
	int opponent_side = engine->w * opponent;
	if (engine->player[opponent].time < u->racket_t){
	    engine->player[opponent].racket = u->racket;
	    engine->player[opponent].time = u->racket_t;
	}
	if (engine->player[engine->self].score < u->score)
	    engine->player[engine->self].score = u->score;
	if (engine->ball.t < u->ball_t
	    && abs(u->ball_x - opponent_side) <= engine->w / 2){
	    engine->ball.x = u->ball_x;
	    engine->ball.y = u->ball_y;
	    engine->ball.v = u->ball_v;
	    engine->ball.d = u->ball_d;
	    engine->ball.t = u->ball_t;
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
#define LAG 0.005
#define RACKET_COLLISION(out, ball, side)\
    (out || abs(ball[0] - engine->w / 2) > abs((side) - engine->w /2))
#define WALL_COLLISION(ball) (ball[1] < 0 || ball[1] > engine->h)
#define LIMIT_COLLISION(ball) (ball[0] < -MARGIN ||			\
			       ball[0] > engine->w + MARGIN ||		\
			       ball[1] < -MARGIN ||			\
			       ball[1] > engine->h + MARGIN)
static double shift(double d, int alpha){
    return d;
}
static void ballCollision(){

    if(engine->time <= engine->ball.t)
	return;

    static int out = 0;
    double ball[2];
    moveBall(ball, engine->time);
    int side = engine->w * engine->self;
    double wall = (ball[1] < 0 ? 0 : engine->w);

    if (LIMIT_COLLISION(ball)){

	//resetting ball
	engine->ball.t = engine->time + 2;
	engine->ball.x = ( engine->w - BALL_SIZE )/ 2;
	engine->ball.y = ( engine->h - BALL_SIZE )/ 2;
	engine->ball.v = BALL_V / 2;
	engine->ball.d = M_PI;
	out = 0;
    } else if(WALL_COLLISION(ball) && !RACKET_COLLISION(out, ball, side)){

	//calculating collision time
	double time = engine->ball.t +
	    abs(wall - engine->ball.y) / (engine->ball.v * sin(engine->ball.d));
	//changing ball direction and origin
	moveBall(ball, time);
	engine->ball.x = ball[0];
	engine->ball.y = ball[1];
	engine->ball.d *= -1;
	engine->ball.t = time;

    } else if(RACKET_COLLISION(out, ball, side) && !WALL_COLLISION(ball)){

	//getting racket position
	int racket = engine->player[engine->self].racket;
	//calculating collision time
	double time = engine->ball.t +
	    abs(side - engine->ball.x) / (engine->ball.v * cos(engine->ball.d));
	//checking if ball received
	moveBall(ball, time);
	if (abs(ball[1]-racket) > RACKET_X / 2){
	    out = 1;
	    //score a point for the opponent
	    engine->player[1-engine->self].score++;
	} else {
	    //changing ball direction and origin
	    moveBall(ball, time);
	    engine->ball.x = ball[0];
	    engine->ball.y = ball[1];
	    engine->ball.d *= shift(M_PI - engine->ball.d, ball[1]-racket);
	    engine->ball.t = time;
	}
    } else if(WALL_COLLISION(ball) && RACKET_COLLISION(out, ball, side)){

	//getting racket position
	int racket = engine->player[engine->self].racket;
	//calculating collision time
	double timeR = engine->ball.t +
	    abs(side - engine->ball.x) / (engine->ball.v * cos(engine->ball.d));
	double timeW = engine->ball.t +
	    abs(wall - engine->ball.y) / (engine->ball.v * sin(engine->ball.d));
	double time = (timeW<timeR ? timeW : timeR);
	//checking if ball received
	moveBall(ball, time);
	if (abs(ball[1]-racket) > RACKET_X / 2){
	    out = 1;
	    //score a point for the opponent
	    engine->player[1-engine->self].score++;
	} else {
	    //changing ball direction and origin
	    moveBall(ball, time);
	    engine->ball.x = ball[0];
	    engine->ball.y = ball[1];
	    engine->ball.d += M_PI;
	    engine->ball.t = time;
	}
    }
}

void run(int direction){

    if (SDL_LockMutex(mutex)){
	printf("%s: %s\n", __func__, SDL_GetError());
	return;
    } else {

	player p = engine->player[engine->self];

	//update engine time
	engine->time = tick() + LAG;

	//calculate racket shift
	double step = direction * RACKET_V * (engine->time - p.time);

	//move racket
	if(p.racket + (int)step < RACKET_Y / 2){
	    if(p.racket != 0)
		p.racket = RACKET_Y / 2;
	} else if(p.racket + step > engine->h - RACKET_Y / 2){
	    if(p.racket != engine->h - RACKET_Y / 2)
		p.racket = engine->h - RACKET_Y / 2;
	} else
	    p.racket += (int)step;

	//update player time
	p.time = engine->time;

	//run ball collision calculation
	ballCollision();

	//Freeing mutex
	SDL_UnlockMutex(mutex);

	return;
    }
}
