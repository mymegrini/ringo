#include <SDL2/SDL.h>
#include <stdlib.h>

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
