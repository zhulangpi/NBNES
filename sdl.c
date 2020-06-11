#include "sdl.h"
#include <SDL2/SDL.h>

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;

static SDL_Joystick    *joy;

static void js_init(void)
{
    if (SDL_NumJoysticks() > 0){ 
        joy = SDL_JoystickOpen(0);
        if (joy != NULL){
            printf("Opened Joystick 0\n");
            printf("Name: %s\n", SDL_JoystickNameForIndex(0));
            printf("Number of Axes: %d\n", SDL_JoystickNumAxes(joy));
            printf("Number of Buttons: %d\n", SDL_JoystickNumButtons(joy));
            printf("Number of Balls: %d\n", SDL_JoystickNumBalls(joy));
            SDL_JoystickEventState(SDL_ENABLE);
        }else{
            printf("Couldn't open Joystick 0\n");
        }
    }else{
        printf("no joystick!\n");
    }
}

static void js_uninit(void)
{
    // Close if opened
    if (SDL_JoystickGetAttached(joy)) {
        SDL_JoystickClose(joy);
    }
}

void sdl_init(void)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

    window   = SDL_CreateWindow("NBNES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 256, 240, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    texture  = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        256,
        240);

    js_init();
}

void sdl_uninit(void)
{

    js_uninit();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}

