#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <readline/readline.h>
#include <string.h>
#include "pong.h"
#include "gui.h"
#include "render.h"
#include "netcode.h"
#include "engine.h"

#define BLACK 255, 0, 0, 0
#define WHITE 255, 255, 255, 255

/**
 * The window we'll be rendering to
 */
static SDL_Window* window = NULL;       /***< SDL window >*/

/**
 * SDL Renderer
 */
static SDL_Renderer* renderer = NULL;   /***< SDL renderer >*/

/**
 * This function frees textures and destroys the window and renderer
 * @return void
 */
void
closeWindow(){

    //Destroy textures
    destroyTextures();

    //Destroy renderer
    SDL_DestroyRenderer( renderer );
    renderer = NULL;

    //Destroy window
    SDL_DestroyWindow( window );
    window = NULL;

    //Quit SDL subsystems
    SDL_Quit();

    return;
}

/**
 * This function creates an SDL window and renderer
 * @return void
 */
void
launchWindow(){

    //Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
	printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	return;
    }

    //Create window
    window = SDL_CreateWindow("pong",
			      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			      WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN );

    if( window == NULL ) {
	printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError());
	return;
    }

    //set icon
    setIcon(window);

    //get Renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC |
				  SDL_RENDERER_ACCELERATED);

    if( renderer == NULL ) {
	printf( "Renderer could not be created! SDL_Eroor: %s\n",
		SDL_GetError());
	return;
    }

    //Set draw color
    SDL_SetRenderDrawColor(renderer, WHITE);

    //load asset Textures
    if (loadTextures(renderer))
	closeWindow();

    return;
}

/**
 * This function handles SDL events
 * @return 0 except on quit where it returns 1
 */
int
handleEvents(){

    SDL_Event evt;

    while(SDL_PollEvent(&evt)) {
	switch(evt.type){
	case SDL_QUIT :
	    return QUIT; // nonzero value to break out of loop
	case SDL_KEYUP :
	    if(evt.key.keysym.sym == SDLK_ESCAPE)
		return QUIT;
	}
    }

    if(engineState()){
	const Uint8* state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_UP] && !state[SDL_SCANCODE_DOWN])
	    return UP;
	else if (state[SDL_SCANCODE_DOWN] && !state[SDL_SCANCODE_UP])
	    return DOWN;
	else
	    return STILL;
    } else
	return OFF;
}

int
launchPong(int argc, char **argv) {

    launchWindow();
    if(window == NULL)
	return 1;

    loginPong();

    int e;
    
    //event loop    
    while((e = handleEvents()) != QUIT){
	simulate(renderer, e);
	if (e != OFF)
	    sendUpdate();
    }
    
    //quitting
    closeWindow();
    logoutPong();

    return 0;
}

/**
 * This function quits pong
 */
void
quitPong(){

    closeWindow();
    logoutPong();
    quitEngine();
    return;
}
