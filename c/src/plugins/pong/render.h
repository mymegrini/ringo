#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>

void loadTextures(SDL_Renderer* renderer);
void destroyTextures();
void renderLogo(SDL_Renderer* renderer);

#endif
