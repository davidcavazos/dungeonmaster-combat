#include "ai.hpp"

#include <cstdio>
#include "game.hpp"

using namespace std;

const int AI_UPDATE = 10; // in frames

void bresenham(Game& g) {
  // try a random movement until a valid move
  while (!g.move_random()) {
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
