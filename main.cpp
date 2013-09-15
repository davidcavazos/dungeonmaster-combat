#include <cstdio>
#include <cstdlib>
#include <SDL/SDL.h>
#include "config.hpp"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_DEPTH = 32;

const Uint32 SDL_INIT_FLAGS = SDL_INIT_VIDEO;
const Uint32 SDL_VIDEO_FLAGS = SDL_HWSURFACE | SDL_ANYFORMAT | SDL_DOUBLEBUF |
                               SDL_RESIZABLE;

const int FRAME_CAP = 60;
const int FRAME_CAP_MS = 1000 / FRAME_CAP;

int main(int, char**) {
  printf("Dungeon Master v%d.%d\n", VERSION_MAJOR, VERSION_MINOR);
  puts("Designed and programmed by: David Cavazos\n");

  SDL_Surface* screen;
  int screen_w = SCREEN_WIDTH;
  int screen_h = SCREEN_HEIGHT;
  int screen_d = SCREEN_DEPTH;

  puts("Initializing SDL");
  if (SDL_Init(SDL_INIT_FLAGS) != 0) {
    fprintf(stderr, "Error initializing SDL: flags=0x%8x\n", SDL_INIT_FLAGS);
    exit(EXIT_FAILURE);
  }
  screen = SDL_SetVideoMode(screen_w, screen_h, screen_d, SDL_VIDEO_FLAGS);
  if (screen == NULL) {
    fprintf(stderr, "Error setting video mode: %dx%dx%d, flags=0x%8x\n",
            screen_w, screen_h, screen_d, SDL_VIDEO_FLAGS);
    SDL_Quit();
    exit(EXIT_FAILURE);
  }

  puts("Running game loop");
  SDL_Event event;
  char buffer[64];
  Uint32 start_time;
  Uint32 delta_time;
  bool is_running = true;
  while (is_running) {
    start_time = SDL_GetTicks();

    // process events
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        is_running = false;
        break;

      case SDL_KEYDOWN:
        // event.key.keysym.sym
        break;

      case SDL_KEYUP:
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
          is_running = false;
          break;
        case SDLK_SPACE:
          break;
        case SDLK_w:
          break;
        case SDLK_a:
          break;
        case SDLK_s:
          break;
        case SDLK_d:
          break;
        case SDLK_j:
          break;
        case SDLK_k:
          break;
        default:
          break;
        }
        break;

      case SDL_MOUSEBUTTONDOWN:
        // event.button.button
        break;

      case SDL_MOUSEBUTTONUP:
        // event.button.button
        break;

      case SDL_MOUSEMOTION:
        // event.motion.x
        // event.motion.y
        // event.motion.xrel
        // event.motion.yrel
        break;

      default:
        // ignore other events
        break;
      }
    }

    // draw to screen
    SDL_Flip(screen);

    // control framerate
    delta_time = SDL_GetTicks() - start_time;
    sprintf(buffer, "Dungeon Master - %02u ms", delta_time);
    SDL_WM_SetCaption(buffer, buffer);
    if (FRAME_CAP_MS > delta_time) {
      SDL_Delay(FRAME_CAP_MS - delta_time);
    }
  }

  puts("Quitting SDL");
  SDL_Quit();
  return EXIT_SUCCESS;
}
