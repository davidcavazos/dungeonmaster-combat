#include "ai.hpp"

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <deque>
#include <map>
#include <vector>
#include "device.hpp"
#include "game.hpp"

using namespace std;

int g_ai_update = 10; // in frames

const int LOW_AI_OBSTACLE = 5;
const int LOW_AI_MEMORY = 200;

// low intelligence AI
map<size_t, vector<vector<int> > > g_ch_map_flags;
typedef struct {
  int x;
  int y;
} pos_x;
map<size_t, deque<pos_x> > g_ch_map_stack;

void create_character_ai(Game& g, size_t idx) {
  const character& ch = g.characters[idx];
  if (ch.stats.intelligence <= 5) {
    auto it = g_ch_map_flags.find(idx);
    if (it == g_ch_map_flags.end()) {
      vector<vector<int> > map_flags(g.map.size());
      for (size_t i = 0; i < map_flags.size(); ++i) {
        map_flags[i].resize(g.map[0].size());
      }
      g_ch_map_flags.insert(make_pair(idx, map_flags));
      g_ch_map_stack.insert(make_pair(idx, deque<pos_x>()));
      puts("Created low intelligence AI");
    }
  }
}

void delete_character_ai(Game& g, size_t idx) {
  const character& ch = g.characters[idx];
  if (ch.stats.intelligence <= 5) {
    auto it1 = g_ch_map_flags.find(idx);
    if (it1 != g_ch_map_flags.end()) {
      g_ch_map_flags.erase(it1);
    }
    auto it2 = g_ch_map_stack.find(idx);
    if (it2 != g_ch_map_stack.end()) {
      g_ch_map_stack.erase(it2);
    }
    puts("Deleted low intelligence AI");
  }
}

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
  auto& map_flags = g_ch_map_flags.find(g.turns[0])->second;
  auto& memstack = g_ch_map_stack.find(g.turns[0])->second;

  int x0 = ch1.pos.x;
  int y0 = ch1.pos.y;
  int destx = ch2.pos.x;
  int desty = ch2.pos.y;

  int dx = abs(destx - x0);
  int dy = abs(desty - y0);
  int err = (dx > dy ? dx : -dy) / 2;

  dx = err > -dx ? destx > x0 ? 1 : -1 : 0;
  dy = err <  dy ? desty > y0 ? 1 : -1 : 0;

  // try to move in straight line
  bool moved = false;
  if (g.can_move(dx, dy, false)) {
    if (rand() % LOW_AI_OBSTACLE >= map_flags[y0+dy][x0+dx]) {
      moved = g.move(dx, dy);
    }
  }
  // forget oldest memory
  static bool is_forgetting = false;
  if (is_forgetting && !memstack.empty()) {
    auto pos = memstack.back();
    --map_flags[pos.y][pos.x];
    memstack.pop_back();
    is_forgetting = false;
  }
  if (moved) {
    return;
  }

  // try to move randomly
  vector<int> neighbors = {0,1,2,3,5,6,7,8};
  random_shuffle(neighbors.begin(), neighbors.end());
  for (size_t i = 0; i < neighbors.size(); ++i) {
    dx = neighbors[i] % 3 - 1;
    dy = neighbors[i] / 3 - 1;
    if (g.can_move(dx, dy, false)) {
      if (rand() % LOW_AI_OBSTACLE >= map_flags[y0+dy][x0+dx]) {
        moved = g.move(dx, dy);
        if (moved) {
          ++map_flags[y0][x0];
          // stack to memory
          memstack.push_back({x0, y0});
          return;
        }
      }
    }
  }
  if (!moved) {
    is_forgetting = true;
  }
}

void process_ai(Game& g) {
  static int ai_frame = 0;
  bool can_take_actions = false;
  vector<size_t> list;
  character& ch = g.characters[g.turns[0]];
  if (!ch.is_playable && ai_frame++ >= g_ai_update) {
    ai_frame = 0;

    srand(clock());
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
    } else if (g.move_limit == 0) {
      g.end_turn();
    }
  }
}

void draw_ai(Device& d, Game& g) {
  size_t idx = g.turns[0];
  const character& ch = g.characters[idx];
  if (ch.is_playable) {
    return;
  }

  if (ch.stats.intelligence <= 5) {
    auto it = g_ch_map_flags.find(idx);
    if (it != g_ch_map_flags.end()) {
      auto& map_flags = it->second;
      for (size_t y = 0; y < map_flags.size(); ++y) {
        for (size_t x = 0; x < map_flags[0].size(); ++x) {
          if (!g.materials[g.map[y][x]].is_walkable) {
            continue;
          }
          d.draw_text(d.pos_x(g,x), d.pos_y(g,y), to_string(map_flags[y][x]));
        }
      }
    }
  }
}
