#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <readline/readline.h>
#include "pong.h"
#include "gui.h"
#include "render.h"
#include "netcode.h"
#include "engine.h"

#define WHITE 255, 255, 255, 255
#define STEP 5

/**
 * The window we'll be rendering to
 */
static SDL_Window* window = NULL;       /***< SDL window >*/

/**
 * SDL Renderer
 */
static SDL_Renderer* renderer = NULL;   /***< SDL renderer >*/

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
			      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			      WINDOW_WIDTH, WINDOW_HEIGHT,
			      SDL_WINDOW_SHOWN );
  
    if( window == NULL ) {
	printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError());
	return;
    }

    //set icon
    setIcon(window);
    
    //get Renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC
				  | SDL_RENDERER_ACCELERATED);

    if( renderer == NULL ) {
	printf( "Renderer could not be created! SDL_Eroor: %s\n",
		SDL_GetError());
	return;
    }

    //Set draw color
    SDL_SetRenderDrawColor(renderer, WHITE);
    
    //load asset Textures
    loadTextures(renderer);

    //render splash screen
    renderLogo(renderer);
    
    return;
}

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
 * This function handles SDL events
 * @return 0 except on quit where it returns 1
 */
int
handleEvents(){
    
    SDL_Event evt;
    
    while(SDL_PollEvent(&evt)) {
	switch(evt.type){
	case SDL_QUIT :
	    return 1; // nonzero value to break out of loop
	case SDL_KEYDOWN :
	    if(evt.key.keysym.sym == SDLK_ESCAPE)
		return 1;
	}
    }

    const Uint8* state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_UP]){
	if (moveRacket(-STEP))
	    sendUpdate();
    }
    if (state[SDL_SCANCODE_DOWN]){
	if (moveRacket(STEP))
	    sendUpdate();
    }
    return 0;
}

int
launchPong(int argc, char **argv) {
    
    launchWindow();
    loginPong();
	
    //event loop
    while(!handleEvents())
	render(renderer);

    //quitting
    quitPong();
    return 0;
}

/**
 * This function quits pong
 */
void
quitPong(){

    closeWindow();
    return;
}
