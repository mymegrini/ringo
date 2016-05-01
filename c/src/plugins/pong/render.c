#include <SDL2/SDL.h>
#include <stdlib.h>
#include "engine.h"
#include "pong.h"

#define DATA_PATH "c/src/plugins/pong/data/"
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
void
loadTextures(SDL_Renderer* renderer){

    loadTexture(renderer, LOGO_BMP, &logoTexture);
    if(!logoTexture)
	puts(SDL_GetError());
    loadTexture(renderer, FIELD_BMP, &fieldTexture);
    if(!fieldTexture)
	puts(SDL_GetError());
    loadTexture(renderer, SKULL_BMP, &skullTexture);
    if(!skullTexture)
	puts(SDL_GetError());
    loadTexture(renderer, TROPHY_BMP, &trophyTexture);
    if(!trophyTexture)
	puts(SDL_GetError());
    
    for(int i = 0; i<10; i++){
	char digit_bmp[256] = { 0 };
	sprintf(digit_bmp, "%s%d.bmp", DATA_PATH, i);
	loadTexture(renderer, digit_bmp, digitTexture + i);
	if(!digitTexture[i])
	    puts(SDL_GetError());
    }
    return;
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
    
/**
 * This function creates the splash window
 * @return void
 */
void
renderLogo(SDL_Renderer* renderer){

    if (logoTexture == NULL)
	loadTextures(renderer);
    
    SDL_RenderCopy(renderer, logoTexture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_Delay(500);
    return;
}

/**
 * This function takes care of rendering the rackets
 */
static void
renderRackets(SDL_Renderer* renderer, int y1, int y2){

    if(y1>-1){
	SDL_Rect racket1 = { X(-RACKET_X), Y(y1), RACKET_X, RACKET_Y };      
	SDL_RenderDrawRect(renderer, &racket1);
    }
    if(y2>-1){
	SDL_Rect racket2 = { X(FIELD_X), Y(y2), RACKET_X, RACKET_Y };
	SDL_RenderDrawRect(renderer, &racket2);
    }
    return;
}

/**
 * This function takes care of rendering the score
 */
static void
renderScore(SDL_Renderer* renderer, int s1, int s2){

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

    SDL_Rect ball = { X(x), Y(y), BALL_SIZE, BALL_SIZE };		      
    SDL_RenderFillRect(renderer, &ball);
    return;
}
    

/**
 * This function renders the current state of the game
 */
void
render(SDL_Renderer* renderer){

    state s;
    getState(&s);
    if(s.available){
	
	if (fieldTexture == NULL)
	loadTextures(renderer);
	
	SDL_RenderCopy(renderer, fieldTexture, NULL, NULL);
	renderScore(renderer, s.score1, s.score2);
	renderRackets(renderer, s.racket1, s.racket2);
	renderBall(renderer, s.ballx, s.bally);

	SDL_RenderPresent(renderer);
	SDL_Delay(100);
    }
    
    return;
}
