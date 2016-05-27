#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include "engine.h"
#include "pong.h"

//#define FRAME_RATE

#define DATA_PATH PONG_PATH "data/"
#define LOGO_BMP DATA_PATH "logo.bmp"
#define ICON_BMP DATA_PATH "icon.bmp"
#define FIELD_BMP DATA_PATH "field.bmp"
#define SKULL_BMP DATA_PATH "skull.bmp"
#define TROPHY_BMP DATA_PATH "trophy.bmp"

/**
 * Asset textures
 */
static SDL_Texture* logoTexture = NULL;
static SDL_Texture* fieldTexture = NULL;
static SDL_Texture* skullTexture = NULL;
static SDL_Texture* trophyTexture = NULL;
static SDL_Texture* digitTexture[10] = { NULL };

/**
 * This function sets window icon
 */
void
setIcon(SDL_Window* window){

    SDL_Surface* icon = SDL_LoadBMP(ICON_BMP);
    SDL_SetWindowIcon(window, icon);
    SDL_FreeSurface(icon);
    return;
}

/**
 * This function loads one file into an SDL texture
 * @param file file path
 * @param texture SDL texture
 * @return void
 */
static void
loadTexture(SDL_Renderer* renderer, const char* file, SDL_Texture** texture){

    SDL_Surface* surface = SDL_LoadBMP( file );
    if (surface) {
	*texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	return;
    } else {
	printf( "%s could not be loaded : %s\n", file, SDL_GetError());
	return;
    }
}

/**
 * This function loads all asset textures
 * @return void
 */
int
loadTextures(SDL_Renderer* renderer){

    int r = 0;

    loadTexture(renderer, LOGO_BMP, &logoTexture);
    if(!logoTexture){
	puts(SDL_GetError());
	r++;
    }
    loadTexture(renderer, FIELD_BMP, &fieldTexture);
    if(!fieldTexture){
	puts(SDL_GetError());
	r++;
    }
    loadTexture(renderer, SKULL_BMP, &skullTexture);
    if(!skullTexture){
	puts(SDL_GetError());
	r++;
    }
    loadTexture(renderer, TROPHY_BMP, &trophyTexture);
    if(!trophyTexture){
	puts(SDL_GetError());
	r++;
    }

    for(int i = 0; i<10; i++){
	char digit_bmp[256] = { 0 };
	sprintf(digit_bmp, "%s%d.bmp", DATA_PATH, i);
	loadTexture(renderer, digit_bmp, digitTexture + i);
	if(!digitTexture[i]){
	    puts(SDL_GetError());
	    r++;
	}
    }

    return r;
}

/**
 * This function frees all asset textures
 * @return void
 */
void
destroyTextures(){

    //Free stored asset textures
    SDL_DestroyTexture(logoTexture);
    logoTexture = NULL;
    SDL_DestroyTexture(skullTexture);
    skullTexture = NULL;
    SDL_DestroyTexture(trophyTexture);
    trophyTexture = NULL;
    for(int i = 0; i<10; i++) {
	SDL_DestroyTexture(digitTexture[i]);
	digitTexture[i] = NULL;
    }
}

#ifdef FRAME_RATE
/**
 * This function calculates framerate and frametimes
 */
static void
framerate(){

    double dt;

    static Uint32 t;
    static int counter;
    static int delta;
    static double average;
    static int minimum;
    static int maximum;

    if (counter == 0){

	//initialization
	t = SDL_GetTicks();
	counter++;
    }
    else if (delta < 1000){
	dt = SDL_GetTicks() - (t+delta);
	delta += dt;

	minimum = (minimum == 0 || dt < minimum ? dt : minimum);
	maximum = (dt > maximum ? dt : maximum);
	average = (average * counter + dt) / (counter + 1);
	counter++;
    }
    else if (delta >=1000){
	printf("framerate : %3d\tframetimes : %2.3f (%2d ..%3d)\ttime : %3.3f\n",
	       counter, average, minimum, maximum, tick());
	dt = SDL_GetTicks() - (t+delta);
	t += delta;
	delta = 0;
	minimum = dt;
	maximum = dt;
	counter = 1;
    }
}
#endif

/**
 * This function creates the splash window
 * @return void
 */
void
renderLogo(SDL_Renderer* renderer){
    if (logoTexture == NULL)
	loadTextures(renderer);

    SDL_RenderCopy(renderer, logoTexture, NULL, NULL);

    return;
}

/**
 * This function takes care of rendering the rackets
 */
static void
renderRackets(SDL_Renderer* renderer, int y1, int y2){

    if(y1>-1){
	SDL_Rect racket1 = { X(-RACKET_X), Y(y1 - RACKET_Y / 2),
			     RACKET_X, RACKET_Y };
	SDL_RenderDrawRect(renderer, &racket1);
    }
    if(y2>-1){
	SDL_Rect racket2 = { X(FIELD_W), Y(y2 - RACKET_Y / 2),
			     RACKET_X, RACKET_Y };
	SDL_RenderDrawRect(renderer, &racket2);
    }
    return;
}

/**
 * This function takes care of rendering the score
 */
static void
renderScore(SDL_Renderer* renderer, int s1, int s2){

    if (s1 > 9 || s2 > 9 || s1 < 0 || s2 < 0){
	s1 = 9;
	s2 = 9;
    }
    if (s1 == 9 || s2 == 9){
	SDL_Texture* result1 = (s1 == 9 ? trophyTexture : skullTexture);
	SDL_Texture* result2 = (s2 == 9 ? trophyTexture : skullTexture);
	SDL_Rect center1 = { RESULT1_X, RESULT1_Y, RESULT_W, RESULT_H };
	SDL_Rect center2 = { RESULT2_X, RESULT2_Y, RESULT_W, RESULT_H };

	SDL_RenderCopy(renderer, result1, NULL, &center1);
	SDL_RenderCopy(renderer, result2, NULL, &center2);
    }

    SDL_Rect digit1 = { WINDOW_WIDTH / 2 - 2 * DIGIT_X, Y(0), DIGIT_X, DIGIT_Y};
    SDL_RenderCopy(renderer, digitTexture[s1], NULL, &digit1);

    SDL_Rect digit2 = { WINDOW_WIDTH / 2 + DIGIT_X, Y(0), DIGIT_X, DIGIT_Y};
    SDL_RenderCopy(renderer, digitTexture[s2], NULL, &digit2);

    return;
}

/**
 * This function takes care of rendering the ball
 */
static void
renderBall(SDL_Renderer* renderer, int x, int y){

    SDL_Rect ball = { X(x - BALL_SIZE / 2), Y(y - BALL_SIZE / 2),
		      BALL_SIZE, BALL_SIZE };
    SDL_RenderFillRect(renderer, &ball);
    return;
}


/**
 * This function renders the current state of the game
 */
void
simulate(SDL_Renderer* renderer, int event){


    state s;
    //updating and running engine
    if(event != OFF && event != OFF)
	run(event);
    getState(&s);

    if(s.available){

	if (fieldTexture == NULL)
	    loadTextures(renderer);

	//rendering result
	SDL_RenderCopy(renderer, fieldTexture, NULL, NULL);
	renderScore(renderer, s.score[0], s.score[1]);
	renderRackets(renderer, s.racket[0], s.racket[1]);
	if(s.available != -1) //GAMEOVER
	    renderBall(renderer, s.ball[0], s.ball[1]);
    }
    else
	renderLogo(renderer);

    SDL_RenderPresent(renderer);

    #ifdef FRAME_RATE
    framerate();
    #else
    SDL_Delay(10);
    #endif

    return;
}
