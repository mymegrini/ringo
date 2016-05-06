#ifndef ENGINE_H
#define ENGINE_H

#define ID_SIZE 8

typedef struct {
    int available;
    int score[2];
    int racket[2];
    int ball[2];
} state;

char* self;
char* opponent;

void initEngine();
void quitEngine();
int  engineState();
void playerid(char* id);
void createSession(const char* host, const char* opponent, int self);
void destroySession();
void getState(state* s);
void updateState(const state* s);
int moveRacket(int step);

#endif
