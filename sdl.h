#ifndef _SDL_H
#define _SDL_H

#include <SDL2/SDL.h>

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern SDL_Texture *texture;

extern void sdl_init(void);
extern void sdl_uninit(void);

#endif
