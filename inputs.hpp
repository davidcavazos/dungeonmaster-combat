#ifndef INPUTS_HPP
#define INPUTS_HPP

#include <SDL/SDL.h>
#include "game.hpp"

inline bool process_input(const SDL_Event& e, Device& d, Game& g) {
  switch (e.type) {
  case SDL_QUIT:
    return false;
    break;

  case SDL_KEYDOWN:
    switch (e.key.keysym.sym) {
    case SDLK_j:
      if (d.is_edit_mode) {
        ++d.random_obstacles;
        if (d.random_obstacles > 80) {
          d.random_obstacles = 80;
        }
      }
      break;
    case SDLK_k:
      if (d.is_edit_mode) {
        --d.random_obstacles;
        if (d.random_obstacles < 20) {
          d.random_obstacles = 20;
        }
      }
      break;
    default:
      break;
    }
    break;

  case SDL_KEYUP:
    switch (e.key.keysym.sym) {
    case SDLK_ESCAPE:
      return false;
      break;
    case SDLK_TAB:
      d.is_edit_mode ^= true;
      g.set_focus();
      break;
    case SDLK_RETURN:
      break;
    case SDLK_SPACE:
      if (!d.is_edit_mode) {
        g.end_turn();
      }
      break;
    case SDLK_LSHIFT: case SDLK_RSHIFT:
      break;
    // normal movements
    case SDLK_w: case SDLK_UP:
      if (d.is_edit_mode) {
        --g.focus_y;
      } else {
        g.move_up();
      }
      break;
    case SDLK_a: case SDLK_LEFT:
      if (d.is_edit_mode) {
        --g.focus_x;
      } else {
        g.move_left();
      }
      break;
    case SDLK_s: case SDLK_x: case SDLK_DOWN:
      if (d.is_edit_mode) {
        ++g.focus_y;
      } else {
        g.move_down();
      }
      break;
    case SDLK_d: case SDLK_RIGHT:
      if (d.is_edit_mode) {
        ++g.focus_x;
      } else {
        g.move_right();
      }
      break;
    // diagonal movements
    case SDLK_q:
      if (d.is_edit_mode) {
        --g.focus_x;
        --g.focus_y;
      } else {
        g.move_left_up();
      }
      break;
    case SDLK_e:
      if (d.is_edit_mode) {
        ++g.focus_x;
        --g.focus_y;
      } else {
        g.move_right_up();
      }
      break;
    case SDLK_z:
      if (d.is_edit_mode) {
        --g.focus_x;
        ++g.focus_y;
      } else {
        g.move_left_down();
      }
      break;
    case SDLK_c:
      if (d.is_edit_mode) {
        ++g.focus_x;
        ++g.focus_y;
      } else {
        g.move_right_down();
      }
      break;
    // other keys
    case SDLK_1:
      if (d.is_edit_mode) {
        g.map[g.focus_y][g.focus_x] = 0;
      }
      break;
    case SDLK_2:
      if (d.is_edit_mode) {
        g.map[g.focus_y][g.focus_x] = 1;
      }
      break;
    case SDLK_3:
      if (d.is_edit_mode) {
        g.map[g.focus_y][g.focus_x] = 2;
      }
      break;
    case SDLK_4:
      if (d.is_edit_mode) {
        g.map[g.focus_y][g.focus_x] = 3;
      }
      break;
    case SDLK_r:
      if (d.is_edit_mode) {
        d.randomize_map(g);
      }
      break;
    case SDLK_t:
      if (d.is_edit_mode) {
        d.random_seed = rand();
      }
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
