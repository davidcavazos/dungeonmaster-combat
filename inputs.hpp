#ifndef INPUTS_HPP
#define INPUTS_HPP

#include <SDL/SDL.h>
#include "game.hpp"

inline bool process_input(const SDL_Event& e, Game& g) {
  static size_t player = 0;

  switch (e.type) {
  case SDL_QUIT:
    return false;
    break;

  case SDL_KEYUP:
    switch (e.key.keysym.sym) {
    case SDLK_ESCAPE:
      return false;
      break;
    case SDLK_F1:
      break;
    case SDLK_RETURN:
      break;
    case SDLK_SPACE:
      ++player;
      if (player >= g.characters.size()) {
        player = 0;
      }
      break;
    case SDLK_LSHIFT: case SDLK_RSHIFT:
      break;
    case SDLK_w:
      --g.characters[player].pos.y;
      break;
    case SDLK_a:
      --g.characters[player].pos.x;
      break;
    case SDLK_s:
      ++g.characters[player].pos.y;
      break;
    case SDLK_d:
      ++g.characters[player].pos.x;
      break;
    default:
      break;
    }
    break;

  case SDL_MOUSEBUTTONUP:
    // e.button.button
    break;

  case SDL_MOUSEMOTION:
    // e.motion.x
    // e.motion.y
    // e.motion.xrel
    // e.motion.yrel
    break;

  default:
    // ignore other events
    break;
  }
  return true;
}

#endif // INPUTS_HPP
