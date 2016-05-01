#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "gui.h"
#include "render.h"

/**
 * The window we'll be rendering to
 */
static SDL_Window* window = NULL;       /***< SDL window >*/

/**
 * SDL Renderer
 */
static SDL_Renderer* renderer = NULL;   /***< SDL renderer >*/

/**
 * This function initializes SDL subsystems
 * @return void
 */
void
initPong(){
    
    //Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
	printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	return;
    }
}

/**
 * This function creates an SDL window and renderer
 * @return void
 */
void
launchWindow(){
    
    //Create window
    window = SDL_CreateWindow("pong",
			      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			      WINDOW_WIDTH, WINDOW_HEIGHT,
			      SDL_WINDOW_SHOWN );
  
    if( window == NULL ) {
	printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError());
	return;
    }

    //get Renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC
				  | SDL_RENDERER_ACCELERATED);

    if( renderer == NULL ) {
	printf( "Renderer could not be created! SDL_Eroor: %s\n",
		SDL_GetError());
	return;
    }

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

    return;
}

/**
 * This function shuts SDL subsystems down
 * @return void
 */
void
closePong(){
    
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
	if(evt.type == SDL_QUIT) {

	    quitPong();
	    return 1; // 1 value to break out of loop
	}
    }

    return 0;
}

int
launchPong(int argc, char **argv) {
    
    launchWindow();

    //event loop
    while(!handleEvents());
    return 0;
}

/**
 * This function quits pong
 */
void
quitPong(){

    closeWindow();
    closePong();
    return;
}
