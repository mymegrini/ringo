#include <SDL2/SDL.h>
#include <stdlib.h>


#define DATA_PATH "c/src/plugins/pong/data/"
#define LOGO_BMP DATA_PATH "logo.bmp"
#define SKULL_BMP DATA_PATH "skull.bmp"
#define TROPHY_BMP DATA_PATH "trophy.bmp"

/**
 * Asset textures
 */
static SDL_Texture* logoTexture = NULL;
static SDL_Texture* skullTexture = NULL;
static SDL_Texture* trophyTexture = NULL;
static SDL_Texture* digitTexture[10] = { NULL };

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
    loadTexture(renderer, SKULL_BMP, &skullTexture);
    loadTexture(renderer, TROPHY_BMP, &trophyTexture);
    
    for(int i = 0; i<10; i++){
	char digit_bmp[256] = { 0 };
	sprintf(digit_bmp, "%s%d.bmp", DATA_PATH, i);
	loadTexture(renderer, digit_bmp, digitTexture + i);
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
    return;
}
