#include "game.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>
#include "device.hpp"
#include "material.hpp"

using namespace std;

const char* DELIM = ", \t";

Game::Game(Device& dev, const string& mat_file, const string& map_file,
           const string& ch_file, const string& en_file) {
  FILE* f;
  char buffer[1024];

  material mat;
  puts("Reading materials");
  f = fopen(mat_file.c_str(), "r");
  if (f == NULL) {
    fprintf(stderr, "Error opening file: %s\n", mat_file.c_str());
    exit(EXIT_FAILURE);
  }
  while (fgets(buffer, sizeof(buffer), f) != NULL) {
    if (buffer[0] == '#') { // comment
      continue;
    }
    mat.name = strtok(buffer, DELIM);
    mat.image = dev.load_image(strtok(NULL, DELIM));
    mat.pos_x = atoi(strtok(NULL, DELIM));
    mat.pos_y = atoi(strtok(NULL, DELIM));
    mat.n_x = atoi(strtok(NULL, DELIM));
    mat.n_y = atoi(strtok(NULL, DELIM));
    mat.is_walkable = atoi(strtok(NULL, DELIM));
    materials.push_back(mat);
  }
  fclose(f);

  load_map(map_file);

  puts("Reading characters");
  character ch;
  f = fopen(ch_file.c_str(), "r");
  if (f == NULL) {
    fprintf(stderr, "Error opening file: %s\n", ch_file.c_str());
    exit(EXIT_FAILURE);
  }
  while (fgets(buffer, sizeof(buffer), f) != NULL) {
    if (buffer[0] == '#') { // comment
      continue;
    }
    ch.name = strtok(buffer, DELIM);
    ch.image = dev.load_image(strtok(NULL, DELIM));
    ch.is_playable = atoi(strtok(NULL, DELIM));
    ch.base_start = atof(strtok(NULL, DELIM));
    ch.base_size = atoi(strtok(NULL, DELIM));
    ch.pos.x = atoi(strtok(NULL, DELIM));
    ch.pos.y = atoi(strtok(NULL, DELIM));
    ch.hp = atoi(strtok(NULL, DELIM));
    ch.hp_max = atoi(strtok(NULL, DELIM));
    ch.armor_class = atoi(strtok(NULL, DELIM));
    ch.stats.strength = atoi(strtok(NULL, DELIM));
    ch.stats.dexterity = atoi(strtok(NULL, DELIM));
    ch.stats.constitution = atoi(strtok(NULL, DELIM));
    ch.stats.intelligence = atoi(strtok(NULL, DELIM));
    ch.stats.wisdom = atoi(strtok(NULL, DELIM));
    ch.stats.charisma = atoi(strtok(NULL, DELIM));
    ch.move_limit = atoi(strtok(NULL, DELIM));
    ch.attack_bonus = atoi(strtok(NULL, DELIM));
    ch.critical = atoi(strtok(NULL, DELIM));
    ch.range = atoi(strtok(NULL, DELIM));
    ch.ammo = atoi(strtok(NULL, DELIM));
    ch.damage = atoi(strtok(NULL, DELIM));
    characters.push_back(ch);
  }
  fclose(f);

  // populate turns
  turns.resize(characters.size());
  for (size_t i = 0; i < turns.size(); ++i) {
    turns[i] = i;
  }

  focus_x = characters[turns[0]].pos.x;
  focus_y = characters[turns[0]].pos.y;
  move_limit = characters[turns[0]].move_limit;
  diag_moves = 0;

  puts("Reading enemies");
  f = fopen(en_file.c_str(), "r");
  if (f == NULL) {
    fprintf(stderr, "Error opening file: %s\n", en_file.c_str());
    exit(EXIT_FAILURE);
  }
  while (fgets(buffer, sizeof(buffer), f) != NULL) {
    if (buffer[0] == '#') { // comment
      continue;
    }
    ch.name = strtok(buffer, DELIM);
    ch.image = dev.load_image(strtok(NULL, DELIM));
    ch.is_playable = atoi(strtok(NULL, DELIM));
    ch.base_start = atof(strtok(NULL, DELIM));
    ch.base_size = atoi(strtok(NULL, DELIM));
    ch.hp = atoi(strtok(NULL, DELIM));
    ch.hp_max = atoi(strtok(NULL, DELIM));
    ch.armor_class = atoi(strtok(NULL, DELIM));
    ch.stats.strength = atoi(strtok(NULL, DELIM));
    ch.stats.dexterity = atoi(strtok(NULL, DELIM));
    ch.stats.constitution = atoi(strtok(NULL, DELIM));
    ch.stats.intelligence = atoi(strtok(NULL, DELIM));
    ch.stats.wisdom = atoi(strtok(NULL, DELIM));
    ch.stats.charisma = atoi(strtok(NULL, DELIM));
    ch.move_limit = atoi(strtok(NULL, DELIM));
    ch.attack_bonus = atoi(strtok(NULL, DELIM));
    ch.critical = atoi(strtok(NULL, DELIM));
    ch.range = atoi(strtok(NULL, DELIM));
    ch.ammo = atoi(strtok(NULL, DELIM));
    ch.damage = atoi(strtok(NULL, DELIM));
    enemies.push_back(ch);
  }
  fclose(f);
}

void Game::load_map(const string& file) {
  char buffer[1024];
  char* tok;

  puts("Reading map");
  FILE* f = fopen(file.c_str(), "r");
  if (f == NULL) {
    fprintf(stderr, "Error opening file: %s\n", file.c_str());
    return;
  }
  // read first row and determine number of columns
  fgets(buffer, sizeof(buffer), f);
  map.resize(1);
  map[0].resize(0);
  tok = strtok(buffer, DELIM);
  while (tok != NULL) {
    map[0].push_back(atoi(tok));
    tok = strtok(NULL, DELIM);
  }

  // read rest of map, incomplete information will be filled with material 0
  while (fgets(buffer, sizeof(buffer), f) != NULL) {
    map.resize(map.size() + 1);
    map.back().resize(map[0].size());
    tok = strtok(buffer, DELIM);
    for (size_t i = 0; i < map[0].size() && tok != NULL; ++i) {
      map.back()[i] = atoi(tok);
      tok = strtok(NULL, DELIM);
    }
  }
  fclose(f);
}

character Game::generate_enemy(size_t idx, int x, int y) {
  static int count = 0;
  character ch;
  ch = enemies[idx];
  ch.name += to_string(++count);
  ch.pos.x = x;
  ch.pos.y = y;
  return ch;
}

bool Game::is_tile_occupied(int x, int y) {
  for (size_t i = 0; i < characters.size(); ++i) {
    const character& ch = characters[i];
    if (ch.pos.x == x && ch.pos.y == y) {
      return true;
    }
  }
  return false;
}

void Game::set_focus() {
  const int DIST = 1;
  int x = characters[turns[0]].pos.x;
  int y = characters[turns[0]].pos.y;

  if (x - focus_x > DIST) {
    focus_x = x - DIST;
  } else if (focus_x - x > DIST) {
    focus_x = x + DIST;
  }

  if (y - focus_y > DIST) {
    focus_y = y - DIST;
  } else if (focus_y - y > DIST) {
    focus_y = y + DIST;
  }
}

void Game::end_turn() {
  size_t temp = turns[0];
  memmove(&turns[0], &turns[1], (turns.size() - 1) * sizeof(turns[0]));
  turns.back() = temp;
  focus_x = characters[turns[0]].pos.x;
  focus_y = characters[turns[0]].pos.y;
  move_limit = characters[turns[0]].move_limit;
  diag_moves = 0;
}

bool Game::move_up() {
  character& ch = characters[turns[0]];
  if (move_limit > 0 &&
      ch.pos.y > 0 &&
      materials[map[ch.pos.y-1][ch.pos.x]].is_walkable &&
      !is_tile_occupied(ch.pos.x, ch.pos.y - 1)) {
    --move_limit;
    --ch.pos.y;
    set_focus();
    return true;
  }
  return false;
}

bool Game::move_down() {
  character& ch = characters[turns[0]];
  if (move_limit > 0 &&
      ch.pos.y < int(map.size()) - 1 &&
      materials[map[ch.pos.y+1][ch.pos.x]].is_walkable &&
      !is_tile_occupied(ch.pos.x, ch.pos.y + 1)) {
    --move_limit;
    ++ch.pos.y;
    set_focus();
    return true;
  }
  return false;
}

bool Game::move_left() {
  character& ch = characters[turns[0]];
  if (move_limit > 0 &&
      ch.pos.x > 0 &&
      materials[map[ch.pos.y][ch.pos.x-1]].is_walkable &&
      !is_tile_occupied(ch.pos.x - 1, ch.pos.y)) {
    --move_limit;
    --ch.pos.x;
    set_focus();
    return true;
  }
  return false;
}

bool Game::move_right() {
  character& ch = characters[turns[0]];
  if (move_limit > 0 &&
      ch.pos.x < int(map[0].size()) - 1 &&
      materials[map[ch.pos.y][ch.pos.x+1]].is_walkable &&
      !is_tile_occupied(ch.pos.x + 1, ch.pos.y)) {
    --move_limit;
    ++ch.pos.x;
    set_focus();
    return true;
  }
  return false;
}

bool Game::move_right_up() {
  character& ch = characters[turns[0]];
  int moves = diag_moves++ % 2 + 1;
  if (move_limit >= moves &&
      ch.pos.x < int(map[0].size()) - 1 &&
      ch.pos.y > 0 &&
      materials[map[ch.pos.y-1][ch.pos.x+1]].is_walkable &&
      !is_tile_occupied(ch.pos.x + 1, ch.pos.y - 1)) {
    move_limit -= moves;
    ++ch.pos.x;
    --ch.pos.y;
    set_focus();
    return true;
  }
  return false;
}

bool Game::move_left_up() {
  character& ch = characters[turns[0]];
  int moves = diag_moves++ % 2 + 1;
  if (move_limit >= moves &&
      ch.pos.x > 0 &&
      ch.pos.y > 0 &&
      materials[map[ch.pos.y-1][ch.pos.x-1]].is_walkable &&
      !is_tile_occupied(ch.pos.x - 1, ch.pos.y - 1)) {
    move_limit -= moves;
    --ch.pos.x;
    --ch.pos.y;
    set_focus();
    return true;
  }
  return false;
}

bool Game::move_right_down() {
  character& ch = characters[turns[0]];
  int moves = diag_moves++ % 2 + 1;
  if (move_limit >= moves &&
      ch.pos.x < int(map[0].size()) - 1 &&
      ch.pos.y < int(map.size()) - 1 &&
      materials[map[ch.pos.y+1][ch.pos.x+1]].is_walkable &&
      !is_tile_occupied(ch.pos.x + 1, ch.pos.y + 1)) {
    move_limit -= moves;
    ++ch.pos.x;
    ++ch.pos.y;
    set_focus();
    return true;
  }
  return false;
}

bool Game::move_left_down() {
  character& ch = characters[turns[0]];
  int moves = diag_moves++ % 2 + 1;
  if (move_limit >= moves &&
      ch.pos.x > 0 &&
      ch.pos.y < int(map.size()) - 1 &&
      materials[map[ch.pos.y+1][ch.pos.x-1]].is_walkable &&
      !is_tile_occupied(ch.pos.x - 1, ch.pos.y + 1)) {
    move_limit -= moves;
    --ch.pos.x;
    ++ch.pos.y;
    set_focus();
    return true;
  }
  return false;
}

bool Game::move_random() {
  srand(clock());
  switch (rand() % 8) {
  case 0:
    return move_up();
  case 1:
    return move_down();
  case 2:
    return move_left();
  case 3:
    return move_right();
  case 4:
    return move_right_up();
  case 5:
    return move_right_down();
  case 6:
    return move_left_up();
  case 7:
    return move_left_down();
  default:
    break;
  }
  return false;
}

vector<size_t> Game::attack_range() {
  vector<size_t> list;
  const character& ch1 = characters[turns[0]];
  for (size_t i = 0; i < characters.size(); ++i) {
    // ignore self
    if (i == turns[0]) {
      continue;
    }
    const character& ch2 = characters[i];
    // no friendly fire
    if (ch1.is_playable == ch2.is_playable) {
      continue;
    }

    double dist_x = abs(double(ch1.pos.x) - ch2.pos.x);
    double dist_y = abs(double(ch1.pos.y) - ch2.pos.y);
    int dist = pow(dist_x * dist_x + dist_y * dist_y, 0.5) + 0.5;
    if (dist <= ch1.range) {
      list.push_back(i);
    }
  }
  return list;
}

void Game::attack(size_t i) {
  const character& ch1 = characters[turns[0]];
  character& ch2 = characters[i];
  int att_mod = 1;
  if (ch1.stats.strength > 18) {
    att_mod = 4;
  }
  srand(clock());
  if (rand() % 20 + ch1.attack_bonus < ch2.armor_class) {
    printf("%s's attack missed\n", ch1.name.c_str());
    return;
  }
  int damage = rand() % ch1.damage + att_mod;
  ch2.hp -= damage;
  printf("%s attacked %s for %d damage\n", ch1.name.c_str(), ch2.name.c_str(),
         damage);

  // if character attacked has no more HP, remove it
  if (ch2.hp <= 0) {
    printf("%s died\n", ch2.name.c_str());
    characters.erase(characters.begin() + i);
    for (size_t j = 0; j < turns.size(); ++j) {
      if (turns[j] == i) {
        turns.erase(turns.begin() + j);
      }
    }
    for (size_t j = 0; j < turns.size(); ++j) {
      if (turns[j] > i) {
        --turns[j];
      }
    }
  }
}
