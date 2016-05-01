#ifndef ENGINE_H
#define ENGINE_H

#define ID_SIZE 8

typedef struct {
    int available;
    int score1;
    int score2;
    int racket1;
    int racket2;
    int ballx;
    int bally;
} state;
    
char* self;
char* opponent;

int  engineState();
void playerid(char* id);
void createSession(const char* host, const char* opponent, int self);
void destroySession();
void getState(state* s);
void moveRacket(int step);

#endif
