#ifndef ENGINE_H
#define ENGINE_H

#define ID_SIZE 8

typedef struct {
    int available;
    int score[2];
    int racket[2];
    int ball[2];
} state;

typedef struct {
    int state;
    int score;         /**< opponent's score >*/
    int racket;        /**< player's racket position >*/
    double racket_t;   /**< player's racket position time >*/
    int ball_x;
    int ball_y;
    int ball_v;
    double ball_d;
    double ball_t;
} update;    
    
char* self;
char* opponent;

double tick();
void initEngine();
void quitEngine();
int engineState();
void playerid(char* id);
void createSession(const char* host, const char* opponent,
		   int self, double* time);
void destroySession();
void getState(state* s);
void getUpdate(update* u);
void updateState(const update* u);
void run(int racket);

#endif
