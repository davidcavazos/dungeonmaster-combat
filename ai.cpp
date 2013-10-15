#include "ai.hpp"

#include <cfloat>
#include <cmath>
#include <cstdio>
#include "game.hpp"

using namespace std;

const int AI_UPDATE = 10; // in frames

size_t nearest_character(Game& g) {
  size_t nearest = 0;
  double min_dist = DBL_MAX;
  const character& ch1 = g.characters[g.turns[0]];
  for (size_t i = 0; i < g.characters.size(); ++i) {
    // ignore self
    if (g.turns[0] == i) {
      continue;
    }
    const character& ch2 = g.characters[i];
    // no friendly fire
    if (ch1.is_playable == ch2.is_playable) {
      continue;
    }

    double dist_x = abs(double(ch1.pos.x) - ch2.pos.x);
    double dist_y = abs(double(ch1.pos.y) - ch2.pos.y);
    double dist = pow(dist_x * dist_x + dist_y * dist_y, 0.5);
    if (dist < min_dist) {
      nearest = i;
      min_dist = dist;
    }
  }
  return nearest;
}

void bresenham(Game& g) {
  size_t nearest = nearest_character(g);
  const character& ch1 = g.characters[g.turns[0]];
  const character& ch2 = g.characters[nearest];
  int x0 = ch1.pos.x;
  int y0 = ch1.pos.y;
  int x1 = ch2.pos.x;
  int y1 = ch2.pos.y;

  bool left = x0 > x1 ? true : false;
  bool down = y0 < y1 ? true : false;

  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int err = (dx > dy ? dx : -dy) / 2;

  bool move_x = err > -dx ? true : false;
  bool move_y = err <  dy ? true : false;

  bool moved = false;
  if (move_x && move_y) {
    if (down) {
      if (left) {
        moved = g.move_left_down();
      } else {
        moved = g.move_right_down();
      }
    } else {
      if (left) {
        moved = g.move_left_up();
      } else {
        moved = g.move_right_up();
      }
    }
  } else if (move_x) {
    if (left) {
      moved = g.move_left();
    } else {
      moved = g.move_right();
    }
  } else if (move_y) {
    if (down) {
      moved = g.move_down();
    } else {
      moved = g.move_up();
    }
  }

  // if reached obstacle
  if (!moved) {
    // try a random movement until a valid move
    while (!g.move_random()) {
    }
  }
}

void process_ai(Game& g) {
  static int ai_frame = 0;
  bool can_take_actions = false;
  vector<size_t> list;
  character& ch = g.characters[g.turns[0]];
  if (!ch.is_playable && ai_frame++ >= AI_UPDATE) {
    ai_frame = 0;

    list = g.attack_range();
    if (list.empty()) {
      // movement depending on intelligence
      if (ch.stats.intelligence <= 5) {
        bresenham(g);
      } else {
        g.end_turn();
      }
    } else {
      g.attack(list[rand() % list.size()]);
      g.end_turn();
    }

    if (can_take_actions) {
    } else if (g.move_limit <= 0) {
      g.end_turn();
    }
  }
}
