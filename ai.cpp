#include "ai.hpp"

#include <cfloat>
#include <climits>
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

const int LOW_AI = 5;
const int LOW_AI_OBSTACLE = 5;

const int MED_AI = 10;
const int MED_AI_OBSTACLE = 10;
const int MED_AI_NUM_BEES = 5;
const int MED_AI_TOTAL_BEE_MOVES = 5;

const int HIGH_AI = 100;
int HIGH_AI_TOTAL_ITERATIONS = 10000;

// common
map<size_t, vector<vector<int> > > g_flag_maps;

// low intelligence AI
typedef struct {
  int x;
  int y;
} pos_t;
map<size_t, deque<pos_t> > g_ch_map_stack;

// medium intelligence AI
const int BEE_ENEMY_IDX = 2;
typedef struct {
  int x;
  int y;
  int last;
} bee_t;
map<size_t, vector<bee_t> > g_bees_map;

// high intelligence AI
//  +---+---+---+   +---+---+---+
//  |ul | u |ur |   | 0 | 1 | 2 |
//  +---+---+---+   +---+---+---+
//  | l | X | r |   | 3 | X | 5 |
//  +---+---+---+   +---+---+---+
//  |dl | d |dr |   | 6 | 7 | 8 |
//  +---+---+---+   +---+---+---+
typedef struct {
  bool visited;
  int ul;
  int u;
  int ur;
  int l;
  int r;
  int dl;
  int d;
  int dr;
} node_t;
typedef struct {
  int x;
  int y;
  int min;
  int max;
  int median;
  deque<short> path;
} graph_data_t;
map<size_t, vector<vector<node_t> > > g_graphs;
map<size_t, graph_data_t> g_graph_datas;
int g_iterations = 0;


extern int g_dijkstra_speed;

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
  // common
  vector<vector<int> > temp(g.map.size());
  auto& flags_map = g_flag_maps.insert(make_pair(idx, temp)).first->second;
  for (size_t i = 0; i < flags_map.size(); ++i) {
    flags_map[i].resize(g.map[0].size());
  }

  // AI dependant
  if (ch.stats.intelligence <= LOW_AI) {
    g_ch_map_stack.insert(make_pair(idx, deque<pos_t>()));
    puts("Created low intelligence AI");
  } else if (ch.stats.intelligence <= MED_AI) {
    const character& ch1 = g.characters[0];
    int x0 = ch1.pos.x;
    int y0 = ch1.pos.y;

    float mult = float(MED_AI_OBSTACLE) / max(g.map.size(), g.map[0].size());
    for (size_t i = 0; i < flags_map.size(); ++i) {
      for (size_t j = 0; j < flags_map[i].size(); ++j) {
        int dx = abs(int(j) - x0) * mult;
        int dy = abs(int(i) - y0) * mult;
        int dist = pow(float(dx * dx + dy * dy), 0.5f);
        flags_map[i][j] = MED_AI_OBSTACLE - min(dist, MED_AI_OBSTACLE);
      }
    }

    vector<bee_t> bees(MED_AI_NUM_BEES);
    for (size_t i = 0; i < bees.size(); ++i) {
      bees[i].x = ch.pos.x;
      bees[i].y = ch.pos.y;
      bees[i].last = 0;
    }
    g_bees_map.insert(make_pair(idx, bees));
    puts("Created medium intelligence AI");
  } else if (ch.stats.intelligence <= HIGH_AI) {
    vector<vector<node_t> > graph(g.map.size());
    for (size_t y = 0; y < graph.size(); ++y) {
      graph[y].resize(g.map[0].size());
      for (size_t x = 0; x < graph[y].size(); ++x) {
        // map range
        if (x >= g.map[0].size()-1) {
          graph[y][x].ur = -1;
          graph[y][x].r  = -1;
          graph[y][x].dr = -1;
        } else if (x <= 0) {
          graph[y][x].ul = -1;
          graph[y][x].l  = -1;
          graph[y][x].dl = -1;
        }
        if (y >= g.map.size()-1) {
          graph[y][x].dl = -1;
          graph[y][x].d  = -1;
          graph[y][x].dr = -1;
        } else if (y <= 0) {
          graph[y][x].ul = -1;
          graph[y][x].u  = -1;
          graph[y][x].ur = -1;
        }

        // obstacles
        if (!g.materials[g.map[y][x]].is_walkable) {
          graph[y][x].ul = -1;
          graph[y][x].u  = -1;
          graph[y][x].ur = -1;
          graph[y][x].l  = -1;
          graph[y][x].r  = -1;
          graph[y][x].dl = -1;
          graph[y][x].d  = -1;
          graph[y][x].dr = -1;
          continue;
        }

        const int init = INT_MAX / 2;
        if (graph[y][x].ul != -1) {
          graph[y][x].ul = g.materials[g.map[y-1][x-1]].is_walkable ? init : -1;
        }
        if (graph[y][x].u  != -1) {
          graph[y][x].u = g.materials[g.map[y-1][x]].is_walkable ? init : -1;
        }
        if (graph[y][x].ur != -1) {
          graph[y][x].ur = g.materials[g.map[y-1][x+1]].is_walkable ? init : -1;
        }
        if (graph[y][x].l != -1) {
          graph[y][x].l = g.materials[g.map[y][x-1]].is_walkable ? init : -1;
        }
        if (graph[y][x].r != -1) {
          graph[y][x].r = g.materials[g.map[y][x+1]].is_walkable ? init : -1;
        }
        if (graph[y][x].dl != -1) {
          graph[y][x].dl = g.materials[g.map[y+1][x-1]].is_walkable ? init : -1;
        }
        if (graph[y][x].d != -1) {
          graph[y][x].d = g.materials[g.map[y+1][x]].is_walkable ? init : -1;
        }
        if (graph[y][x].dr != -1) {
          graph[y][x].dr = g.materials[g.map[y+1][x+1]].is_walkable ? init : -1;
        }
        graph[y][x].visited = false;
      }
    }
    g_graphs.insert(make_pair(idx, graph));
    graph_data_t data;
    data.x = ch.pos.x;
    data.y = ch.pos.y;
    data.min = INT_MAX;
    data.max = 0;
    data.median = INT_MAX / 2;
    g_graph_datas.insert(make_pair(idx, data));
    puts("Created high intelligence AI");
  }
}

void delete_character_ai(Game& g, size_t idx) {
  const character& ch = g.characters[idx];
  auto fm = g_flag_maps.find(idx);
  if (fm != g_flag_maps.end()) {
    g_flag_maps.erase(fm);
  }
  if (ch.stats.intelligence <= LOW_AI) {
    auto it = g_ch_map_stack.find(idx);
    if (it != g_ch_map_stack.end()) {
      g_ch_map_stack.erase(it);
    }
    puts("Deleted low intelligence AI");
  } else if (ch.stats.intelligence <= MED_AI) {
    auto it = g_bees_map.find(idx);
    if (it != g_bees_map.end()) {
      g_bees_map.erase(it);
    }
    puts("Deleted medium intelligence AI");
  } else if (ch.stats.intelligence <= HIGH_AI) {
    auto it = g_graphs.find(idx);
    if (it != g_graphs.end()) {
      g_graphs.erase(it);
    }
    auto it2 = g_graph_datas.find(idx);
    if (it2 != g_graph_datas.end()) {
      g_graph_datas.erase(it2);
    }
    puts("Deleted high intelligence AI");
  }
}

void bresenham_algorithm(Game& g) {
  size_t nearest = nearest_character(g);
  const character& ch1 = g.characters[g.turns[0]];
  const character& ch2 = g.characters[nearest];
  auto& flags_map = g_flag_maps.find(g.turns[0])->second;
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
    if (rand() % LOW_AI_OBSTACLE >= flags_map[y0+dy][x0+dx]) {
      moved = g.move(dx, dy);
    }
  }
  // forget oldest memory
  static bool is_forgetting = false;
  if (is_forgetting && !memstack.empty()) {
    auto pos = memstack.back();
    --flags_map[pos.y][pos.x];
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
        rand() % LOW_AI_OBSTACLE >= flags_map[y0+dy][x0+dx]) {
      moved = g.move(dx, dy);
      if (moved) {
        ++flags_map[y0][x0];
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
  auto& flags_map = g_flag_maps.find(g.turns[0])->second;
  auto& bees = g_bees_map.find(g.turns[0])->second;

  static int bee_moves = 0;
  if (bee_moves < MED_AI_TOTAL_BEE_MOVES) {
    size_t best = 0;
    int best_temp = 0;
    for (size_t i = 0; i < bees.size(); ++i) {
      int x0 = bees[i].x;
      int y0 = bees[i].y;
      if (flags_map[y0][x0] > best_temp) {
        best = i;
        best_temp = flags_map[y0][x0];
      }
    }
    for (size_t i = 0; i < bees.size(); ++i) {
      bees[i].x = bees[best].x;
      bees[i].y = bees[best].y;
    }
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
          } else if (flags_map[y1][x1] > 0) {
            --flags_map[y1][x1];
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
      if (flags_map[y0][x0] > best_temp) {
        best = i;
        best_temp = flags_map[y0][x0];
      }
    }
    g.move(bees[best].x - ch.pos.x, bees[best].y - ch.pos.y);
    bee_moves = 0;
  }
}

void graph_algorithm(Game& g) {
  bool draw_steps = g_dijkstra_speed > 1;

  auto& flags_map = g_flag_maps.find(g.turns[0])->second;
  auto& data = g_graph_datas.find(g.turns[0])->second;
  auto& graph = g_graphs.find(g.turns[0])->second;
  const character& ch = g.characters[g.turns[0]];

  vector<size_t> list;
  while (list.empty()) {
    // check if it reached its destination
    list.resize(0);
    for (size_t i = 0; i < g.characters.size(); ++i) {
      if (i == g.turns[0]) {
        continue;
      }
      const character& ch2 = g.characters[i];
      if (ch.is_playable == ch2.is_playable) {
        continue;
      }

      double dist_x = abs(double(data.x) - ch2.pos.x);
      double dist_y = abs(double(data.y) - ch2.pos.y);
      int dist = pow(dist_x * dist_x + dist_y * dist_y, 0.5) + 0.5;
      if (dist <= ch.range) {
        list.push_back(i);
      }
    }
    static bool finished = false;
    if (!list.empty() && !finished) {
      finished = true;
      return;
    }
    if (finished) {
      finished = false;
      for (size_t y = 0; y < flags_map.size(); ++y) {
        for (size_t x = 0; x < flags_map[y].size(); ++x) {
          flags_map[y][x] = 0;
        }
      }
      static bool first_time = true;
      if (!first_time) {
        data.x = ch.pos.x;
        data.y = ch.pos.y;
        int error = data.median - int(data.path.size());
        for (size_t i = 0; i < data.path.size(); ++i) {
          switch (data.path[i]) {
            case 0:
              graph[data.y][data.x].ul += error;
              --data.x;
              --data.y;
              graph[data.y][data.x].dr += error;
              break;
            case 1:
              graph[data.y][data.x].u += error;
              --data.y;
              graph[data.y][data.x].d += error;
              break;
            case 2:
              graph[data.y][data.x].ur += error;
              ++data.x;
              --data.y;
              graph[data.y][data.x].dl += error;
              break;
            case 3:
              graph[data.y][data.x].l += error;
              --data.x;
              graph[data.y][data.x].r += error;
              break;
            case 5:
              graph[data.y][data.x].r += error;
              ++data.x;
              graph[data.y][data.x].l += error;
              break;
            case 6:
              graph[data.y][data.x].dl += error;
              --data.x;
              ++data.y;
              graph[data.y][data.x].ur += error;
              break;
            case 7:
              graph[data.y][data.x].d += error;
              ++data.y;
              graph[data.y][data.x].u += error;
              break;
            case 8:
              graph[data.y][data.x].dr += error;
              ++data.x;
              ++data.y;
              graph[data.y][data.x].ul += error;
              break;
            default:
              fprintf(stderr, "Error: invalid path direction: %d\n",
                      data.path[i]);
          }
        }
        //printf("path:%zu\tmin:%d\tmax:%d\tmedian:%d\terror:%d\n",
        //       data.path.size(), data.min, data.max, data.median, error);
      }
      first_time = false;
      data.x = ch.pos.x;
      data.y = ch.pos.y;
      data.min = min(data.min, int(data.path.size()));
      data.max = max(data.max, int(data.path.size()));
      data.median = (data.max+data.min) / 2;
      //printf("path:%zu\tmin:%d\tmax:%d\tmedian:%d\n",
      //       data.path.size(), data.min, data.max, data.median);
      data.path.resize(0);
      ++g_iterations;
      printf("%d of %d\n", g_iterations, HIGH_AI_TOTAL_ITERATIONS);
      continue;
    }

    if (g_iterations < HIGH_AI_TOTAL_ITERATIONS) {
      // randomly fill graph
      bool moved = false;
      vector<int> neighbors = {0,1,2,3,5,6,7,8};
      random_shuffle(neighbors.begin(), neighbors.end());
      for (size_t i = 0; i < neighbors.size(); ++i) {
        int dx = neighbors[i] % 3 - 1;
        int dy = neighbors[i] / 3 - 1;
        int x1 = data.x + dx;
        int y1 = data.y + dy;
        if (x1 >= 0 &&
            x1 < int(g.map[0].size()) &&
            y1 < int(g.map.size()) &&
            y1 >= 0 &&
            g.materials[g.map[y1][x1]].is_walkable &&
            !g.is_tile_occupied(x1, y1) &&
            flags_map[y1][x1] == 0)
        {
          moved = true;
          data.x = x1;
          data.y = y1;
          data.path.push_back(neighbors[i]);
          flags_map[data.y][data.x] = 1;
          graph[data.y][data.x].visited = true;
          if (draw_steps) {
            if (data.x - g.focus_x > 1) {
              g.focus_x = data.x - 1;
            } else if (g.focus_x - data.x > 1) {
              g.focus_x = data.x + 1;
            }
            if (data.y - g.focus_y > 1) {
              g.focus_y = data.y - 1;
            } else if (g.focus_y - data.y > 1) {
              g.focus_y = data.y + 1;
            }
            return;
          }
          break;
        }
      }
      if (!moved) {
        for (size_t i = 0; i < neighbors.size(); ++i) {
          int dx = neighbors[i] % 3 - 1;
          int dy = neighbors[i] / 3 - 1;
          int x1 = data.x + dx;
          int y1 = data.y + dy;
          if (x1 >= 0 &&
              x1 < int(g.map[0].size()) &&
              y1 < int(g.map.size()) &&
              y1 >= 0 &&
              g.materials[g.map[y1][x1]].is_walkable &&
              !g.is_tile_occupied(x1, y1))
          {
            /*
            // avoid last visited
            if (!data.path.empty() && neighbors[i] == data.path.back()) {
              continue;
            }
            */
            moved = true;
            data.x = x1;
            data.y = y1;
            data.path.push_back(neighbors[i]);
            flags_map[data.y][data.x] = 1;
            graph[data.y][data.x].visited = true;
            if (draw_steps) {
              if (data.x - g.focus_x > 1) {
                g.focus_x = data.x - 1;
              } else if (g.focus_x - data.x > 1) {
                g.focus_x = data.x + 1;
              }
              if (data.y - g.focus_y > 1) {
                g.focus_y = data.y - 1;
              } else if (g.focus_y - data.y > 1) {
                g.focus_y = data.y + 1;
              }
              return;
            }
            break;
          }
        }
      }
    } else {
      break;
    }
  }

  // dijkstra
  if (g_iterations >= HIGH_AI_TOTAL_ITERATIONS) {
    static deque<pos_t> path;

    static bool first_time = true;
    if (first_time) {
      first_time = false;
      puts("Calculating Dijkstra's shortest path");

      typedef struct {
        bool visited;
        int dist;
        pos_t prev;
      } dijkstra_t;

      // init
      vector<vector<dijkstra_t> > dijkstra(g.map.size());
      for (size_t y = 0; y < dijkstra.size(); ++y) {
        dijkstra[y].resize(g.map[y].size());
        for (size_t x = 0; x < dijkstra[y].size(); ++x) {
          dijkstra[y][x].visited = false;
          dijkstra[y][x].dist = INT_MAX;
          dijkstra[y][x].prev.x = -1;
          dijkstra[y][x].prev.y = -1;
        }
      }
      pos_t cur;
      cur.x = ch.pos.x;
      cur.y = ch.pos.y;
      dijkstra[cur.y][cur.x].visited = true;
      dijkstra[cur.y][cur.x].dist = 0;

      const character& dest_ch = g.characters[nearest_character(g)];
      pos_t dest;
      dest.x = dest_ch.pos.x;
      dest.y = dest_ch.pos.y;

      // iterations
      const int R = ch.range;
      while (cur.x < dest.x-R || cur.x > dest.x+R ||
             cur.y < dest.y-R || cur.y > dest.y+R)
      {
        node_t& n = graph[cur.y][cur.x];
        dijkstra_t& d   = dijkstra[cur.y  ][cur.x  ];
        dijkstra_t& dul = dijkstra[cur.y-1][cur.x-1];
        dijkstra_t& du  = dijkstra[cur.y-1][cur.x  ];
        dijkstra_t& dur = dijkstra[cur.y-1][cur.x+1];
        dijkstra_t& dl  = dijkstra[cur.y  ][cur.x-1];
        dijkstra_t& dr  = dijkstra[cur.y  ][cur.x+1];
        dijkstra_t& ddl = dijkstra[cur.y+1][cur.x-1];
        dijkstra_t& dd  = dijkstra[cur.y+1][cur.x  ];
        dijkstra_t& ddr = dijkstra[cur.y+1][cur.x+1];

        // measure neighbor's distances and get minimum path
        if (n.ul != -1 && !dul.visited && d.dist+n.ul < dul.dist) {
          dul.dist = d.dist+n.ul;
          dul.prev = cur;
        }
        if (n.u != -1 && !du.visited && d.dist+n.u < du.dist) {
          du.dist = d.dist+n.u;
          du.prev = cur;
        }
        if (n.ur != -1 && !dur.visited && d.dist+n.ur < dur.dist) {
          dur.dist = d.dist+n.ur;
          dur.prev = cur;
        }
        if (n.l != -1 && !dl.visited && d.dist+n.l < dl.dist) {
          dl.dist = d.dist+n.l;
          dl.prev = cur;
        }
        if (n.r != -1 && !dr.visited && d.dist+n.r < dr.dist) {
          dr.dist = d.dist+n.r;
          dr.prev = cur;
        }
        if (n.dl != -1 && !ddl.visited && d.dist+n.dl < ddl.dist) {
          ddl.dist = d.dist+n.dl;
          ddl.prev = cur;
        }
        if (n.d != -1 && !dd.visited && d.dist+n.d < dd.dist) {
          dd.dist = d.dist+n.d;
          dd.prev = cur;
        }
        if (n.dr != -1 && !ddr.visited && d.dist+n.dr < ddr.dist) {
          ddr.dist = d.dist+n.dr;
          ddr.prev = cur;
        }

        // find the unvisited node with minimum distance
        int min_dist = INT_MAX;
        pos_t min = cur;
        for (size_t y = 0; y < dijkstra.size(); ++y) {
          for (size_t x = 0; x < dijkstra[y].size(); ++x) {
            dijkstra_t& d = dijkstra[y][x];
            if (!d.visited && d.dist < min_dist) {
              min_dist = d.dist;
              min.x = x;
              min.y = y;
            }
          }
        }

        // visit min
        cur = min;
        dijkstra[cur.y][cur.x].visited = true;
      }
      // backpropagate path
      do {
        path.push_front(cur);
        cur = dijkstra[cur.y][cur.x].prev;
      } while (cur.x >= 0 && cur.y >= 0);
    }

    // move
    g.move(path.front().x-ch.pos.x, path.front().y-ch.pos.y);
    path.pop_front();
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
      if (ch.stats.intelligence <= LOW_AI) {
        bresenham_algorithm(g);
      } else if (ch.stats.intelligence <= MED_AI) {
        bees_algorithm(g);
      } else if (ch.stats.intelligence <= HIGH_AI) {
        graph_algorithm(g);
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
  auto it = g_flag_maps.find(idx);
  if (it == g_flag_maps.end()) {
    fputs("Error: flags map not created", stderr);
    return;
  }
  auto& flags_map = it->second;

  if (ch.stats.intelligence <= LOW_AI) {
    for (size_t y = 0; y < flags_map.size(); ++y) {
      for (size_t x = 0; x < flags_map[0].size(); ++x) {
        if (!g.materials[g.map[y][x]].is_walkable) {
          continue;
        }
        float f = flags_map[y][x];
        Uint8 r = Uint8(f / LOW_AI_OBSTACLE * 255.0f);
        Uint8 b = 255 - r;
        SDL_Color color = {r,0,b,255};
        d.draw_rect(d.pos_x(g,x)+3, d.pos_y(g,y)+3, 57, 57, color);
        d.draw_rect(d.pos_x(g,x)+4, d.pos_y(g,y)+4, 55, 55, color);
        d.draw_rect(d.pos_x(g,x)+5, d.pos_y(g,y)+5, 53, 53, color);
      }
    }
  } else if (ch.stats.intelligence <= MED_AI) {
    for (size_t y = 0; y < flags_map.size(); ++y) {
      for (size_t x = 0; x < flags_map[0].size(); ++x) {
        if (!g.materials[g.map[y][x]].is_walkable) {
          continue;
        }
        float f = flags_map[y][x];
        Uint8 r = Uint8(f / MED_AI_OBSTACLE * 255.0f);
        Uint8 b = 255 - r;
        SDL_Color color = {r,0,b,255};
        d.draw_rect(d.pos_x(g,x)+3, d.pos_y(g,y)+3, 57, 57, color);
        d.draw_rect(d.pos_x(g,x)+4, d.pos_y(g,y)+4, 55, 55, color);
        d.draw_rect(d.pos_x(g,x)+5, d.pos_y(g,y)+5, 53, 53, color);
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
  } else if (ch.stats.intelligence <= HIGH_AI) {
    auto& data = g_graph_datas.find(g.turns[0])->second;
    auto it = g_graphs.find(idx);

    Uint8 c;
    if (it != g_graphs.end()) {
      auto& graph = it->second;
      for (size_t y = 0; y < graph.size(); ++y) {
        for (size_t x = 0; x < graph[0].size(); ++x) {
          if (!g.materials[g.map[y][x]].is_walkable) {
            continue;
          }
          const node_t& node = graph[y][x];
          int cx = d.pos_x(g,x) + 32;
          int cy = d.pos_y(g,y) + 32;

          if (!graph[y][x].visited) {
            continue;
          }

          static int _min = INT_MAX;
          static int _max = INT_MIN;
          if (node.ur > 0 && node.ur < _min)   _min = node.ur;
          if (node.ur > 0 && node.ur > _max)   _max = node.ur;
          if (node.u  > 0 && node.ur < _min)   _min = node.u;
          if (node.u  > 0 && node.ur > _max)   _max = node.u;
          if (node.ul > 0 && node.ur < _min)   _min = node.ul;
          if (node.ul > 0 && node.ur > _max)   _max = node.ul;
          if (node.l  > 0 && node.ur < _min)   _min = node.l;
          if (node.l  > 0 && node.ur > _max)   _max = node.l;
          if (node.r  > 0 && node.ur < _min)   _min = node.r;
          if (node.r  > 0 && node.ur > _max)   _max = node.r;
          if (node.dr > 0 && node.ur < _min)   _min = node.dr;
          if (node.dr > 0 && node.ur > _max)   _max = node.dr;
          if (node.d  > 0 && node.ur < _min)   _min = node.d;
          if (node.d  > 0 && node.ur > _max)   _max = node.d;
          if (node.dl > 0 && node.ur < _min)   _min = node.dl;
          if (node.dl > 0 && node.ur > _max)   _max = node.dl;

          if (node.ur != -1 && graph[y-1][x+1].visited) {
            c = Uint8(float(node.ur-_min) / (_max-_min) * 255.0f);
            d.draw_line(cx+16, cy-16, cx+48, cy-48, {c,c,c,255});
          }

          if (node.r != -1 && graph[y][x+1].visited) {
            c = Uint8(float(node.r-_min) / (_max-_min) * 255.0f);
            d.draw_line(cx+16, cy, cx+48, cy, {c,c,c,255});
          }

          if (node.d != -1 && graph[y+1][x].visited) {
            c = Uint8(float(node.d-_min) / (_max-_min) * 255.0f);
            d.draw_line(cx, cy+16, cx, cy+48, {c,c,c,255});
          }

          if (node.dr != -1 && graph[y+1][x+1].visited) {
            c = Uint8(float(node.dr-_min) / (_max-_min) * 255.0f);
            d.draw_line(cx+16, cy+16, cx+48, cy+48, {c,c,c,255});
          }

          /*
          if (node.ul != -1 && graph[y-1][x-1].visited) {
            c = Uint8(float(node.ul-_min) / (_max-_min) * 255.0f);
            d.draw_line(cx-16, cy-16, cx-48, cy-48, {c,c,c,255});
          }

          if (node.u != -1 && graph[y-1][x].visited) {
            c = Uint8(float(node.u-_min) / (_max-_min) * 255.0f);
            d.draw_line(cx, cy-16, cx, cy-48, {c,c,c,255});
          }

          if (node.l != -1 && graph[y][x-1].visited) {
            c = Uint8(float(node.l-_min) / (_max-_min) * 255.0f);
            d.draw_line(cx-16, cy, cx-48, cy, {c,c,c,255});
          }

          if (node.dl != -1 && graph[y+1][x-1].visited) {
            c = Uint8(float(node.dl-_min) / (_max-_min) * 255.0f);
            d.draw_line(cx-16, cy+16, cx-48, cy+48, {c,c,c,255});
          }
          */

          d.draw_rect(cx-16, cy-16, 32, 32, {255,255,255,255});
        }
      }
      d.draw_rect(d.pos_x(g,data.x)+16, d.pos_y(g,data.y)+16, 32, 32,
                  {255,255,0,255});
    }
  }
}
