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
const int MED_AI_OBSTACLE = 10;
const int MED_AI_NUM_BEES = 5;
const int MED_AI_TOTAL_BEE_MOVES = 5;

// low intelligence AI
map<size_t, vector<vector<int> > > g_ch_map_flags;
typedef struct {
  int x;
  int y;
} pos_x;
map<size_t, deque<pos_x> > g_ch_map_stack;

// medium intelligence AI
const int BEE_ENEMY_IDX = 2;
typedef struct {
  int x;
  int y;
  int last;
} bee_t;
map<size_t, vector<bee_t> > g_bees_map;

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
  } else if (ch.stats.intelligence <= 10) {
    const character& ch1 = g.characters[0];
    int x0 = ch1.pos.x;
    int y0 = ch1.pos.y;

    float mult = float(MED_AI_OBSTACLE) / max(g.map.size(), g.map[0].size());
    auto it = g_ch_map_flags.find(idx);
    if (it == g_ch_map_flags.end()) {
      vector<vector<int> > map_flags(g.map.size());
      for (size_t i = 0; i < map_flags.size(); ++i) {
        map_flags[i].resize(g.map[0].size());
        for (size_t j = 0; j < map_flags[i].size(); ++j) {
          int dx = abs(int(j) - x0) * mult;
          int dy = abs(int(i) - y0) * mult;
          int dist = pow(float(dx * dx + dy * dy), 0.5f);
          map_flags[i][j] = MED_AI_OBSTACLE - min(dist, MED_AI_OBSTACLE);
        }
      }
      g_ch_map_flags.insert(make_pair(idx, map_flags));

      vector<bee_t> bees(MED_AI_NUM_BEES);
      for (size_t i = 0; i < bees.size(); ++i) {
        bees[i].x = ch.pos.x;
        bees[i].y = ch.pos.y;
        bees[i].last = 0;
      }
      g_bees_map.insert(make_pair(idx, bees));
      puts("Created medium intelligence AI");
    }
  }
}

void delete_character_ai(Game& g, size_t idx) {
  const character& ch = g.characters[idx];
  if (ch.stats.intelligence <= 5) {
    auto it1 = g_ch_map_stack.find(idx);
    if (it1 != g_ch_map_stack.end()) {
      g_ch_map_stack.erase(it1);
    }
    auto it2 = g_ch_map_flags.find(idx);
    if (it2 != g_ch_map_flags.end()) {
      g_ch_map_flags.erase(it2);
    }
    puts("Deleted low intelligence AI");
  } else if (ch.stats.intelligence <= 10) {
    auto it1 = g_bees_map.find(idx);
    if (it1 != g_bees_map.end()) {
      g_bees_map.erase(it1);
    }
    auto it2 = g_ch_map_flags.find(idx);
    if (it2 != g_ch_map_flags.end()) {
      g_ch_map_flags.erase(it2);
    }
    puts("Deleted medium intelligence AI");
  }
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
    if (g.can_move(dx, dy, false) &&
        rand() % LOW_AI_OBSTACLE >= map_flags[y0+dy][x0+dx]) {
      moved = g.move(dx, dy);
      if (moved) {
        ++map_flags[y0][x0];
        // stack to memory
        memstack.push_back({x0, y0});
        return;
      }
    }
  }
  if (!moved) {
    is_forgetting = true;
  }
}

void bees_algorithm(Game& g) {
  const character& ch = g.characters[g.turns[0]];
  auto& map_flags = g_ch_map_flags.find(g.turns[0])->second;
  auto& bees = g_bees_map.find(g.turns[0])->second;

  static int bee_moves = 0;
  if (bee_moves < MED_AI_TOTAL_BEE_MOVES) {
    for (size_t i = 0; i < bees.size(); ++i) {
      int x0 = bees[i].x;
      int y0 = bees[i].y;

      vector<int> neighbors = {0,1,2,3,5,6,7,8};
      random_shuffle(neighbors.begin(), neighbors.end());
      for (size_t i = 0; i < neighbors.size(); ++i) {
        // do not go back to the last cell
        if (neighbors[i] == 8 - bees[i].last) {
          continue;
        }
        int dx = neighbors[i] % 3 - 1;
        int dy = neighbors[i] / 3 - 1;
        int x1 = x0 + dx;
        int y1 = y0 + dy;
        if (x1 >= 0 &&
            x1 < int(g.map[0].size()) &&
            y1 < int(g.map.size()) &&
            y1 >= 0) {
          if (g.materials[g.map[y1][x1]].is_walkable &&
              !g.is_tile_occupied(x1, y1)) {
            bees[i].x = x1;
            bees[i].y = y1;
            bees[i].last = neighbors[i];
            break;
          } else if (map_flags[y1][x1] > 0) {
            --map_flags[y1][x1];
          }
        }
      }
    }
    ++bee_moves;
  } else {
    size_t best = 0;
    int best_temp = 0;
    for (size_t i = 0; i < bees.size(); ++i) {
      int x0 = bees[i].x;
      int y0 = bees[i].y;
      if (map_flags[y0][x0] > best_temp) {
        best = i;
        best_temp = map_flags[y0][x0];
      }
    }
    for (size_t i = 0; i < bees.size(); ++i) {
      bees[i].x = bees[best].x;
      bees[i].y = bees[best].y;
    }
    g.move(bees[best].x - ch.pos.x, bees[best].y - ch.pos.y);
    bee_moves = 0;
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
      } else if (ch.stats.intelligence <= 10) {
        bees_algorithm(g);
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

void draw_ai(Device& d, const Game& g) {
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
          float f = map_flags[y][x];
          Uint8 r = Uint8(f / LOW_AI_OBSTACLE * 255.0f);
          Uint8 b = 255 - r;
          SDL_Color color = {r,0,b,255};
          d.draw_rect(d.pos_x(g,x)+3, d.pos_y(g,y)+3, 57, 57, color);
          d.draw_rect(d.pos_x(g,x)+4, d.pos_y(g,y)+4, 55, 55, color);
          d.draw_rect(d.pos_x(g,x)+5, d.pos_y(g,y)+5, 53, 53, color);
        }
      }
    }
  } else if (ch.stats.intelligence <= 10) {
    auto it = g_ch_map_flags.find(idx);
    if (it != g_ch_map_flags.end()) {
      auto& map_flags = it->second;
      for (size_t y = 0; y < map_flags.size(); ++y) {
        for (size_t x = 0; x < map_flags[0].size(); ++x) {
          if (!g.materials[g.map[y][x]].is_walkable) {
            continue;
          }
          float f = map_flags[y][x];
          Uint8 r = Uint8(f / MED_AI_OBSTACLE * 255.0f);
          Uint8 b = 255 - r;
          SDL_Color color = {r,0,b,255};
          d.draw_rect(d.pos_x(g,x)+3, d.pos_y(g,y)+3, 57, 57, color);
          d.draw_rect(d.pos_x(g,x)+4, d.pos_y(g,y)+4, 55, 55, color);
          d.draw_rect(d.pos_x(g,x)+5, d.pos_y(g,y)+5, 53, 53, color);
        }
      }
    }
    auto it2 = g_bees_map.find(idx);
    if (it2 != g_bees_map.end()) {
      auto& bees = it2->second;
      for (size_t i = 0; i < bees.size(); ++i) {
        const auto& b = bees[i];
        d.draw_sprite(d.pos_x(g, b.x) - g.enemies[BEE_ENEMY_IDX].base_start,
                      d.pos_y(g, b.y), g.enemies[BEE_ENEMY_IDX].image);
      }
    }
  }
}
