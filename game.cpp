#include "game.hpp"

#include <cstdio>
#include <cstring>
#include "device.hpp"
#include "material.hpp"

using namespace std;

const char* DELIM = ", ";

Game::Game(Device& dev, const string& mat_file, const string& map_file,
           const string& ch_file) {
  FILE* f;
  char buffer[128];

  material mat;
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

  char* tok;
  f = fopen(map_file.c_str(), "r");
  if (f == NULL) {
    fprintf(stderr, "Error opening file: %s\n", map_file.c_str());
    exit(EXIT_FAILURE);
  }
  // read first row and determine number of columns
  fgets(buffer, sizeof(buffer), f);
  map.resize(1);
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

  // test characters
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
    ch.base_start = atoi(strtok(NULL, DELIM));
    ch.base_size = atoi(strtok(NULL, DELIM));
    ch.pos.x = atoi(strtok(NULL, DELIM));
    ch.pos.y = atoi(strtok(NULL, DELIM));
    ch.stats.strength = atoi(strtok(NULL, DELIM));
    ch.stats.dexterity = atoi(strtok(NULL, DELIM));
    ch.stats.constitution = atoi(strtok(NULL, DELIM));
    ch.stats.intelligence = atoi(strtok(NULL, DELIM));
    ch.stats.wisdom = atoi(strtok(NULL, DELIM));
    ch.stats.charisma = atoi(strtok(NULL, DELIM));
    characters.push_back(ch);
  }
  fclose(f);

  focus_x = map[0].size() / 2;
  focus_y = map.size() / 2;
}
