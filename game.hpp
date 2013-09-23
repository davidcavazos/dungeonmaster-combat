#ifndef GAME_HPP
#define GAME_HPP

#include <vector>
#include "character.hpp"
#include "material.hpp"

class Device;

class Game {
public:
  int focus_x;
  int focus_y;
  int move_limit;
  std::vector<material> materials;
  std::vector<std::vector<size_t> > map;
  std::vector<character> characters;
  std::vector<size_t> turns;

  Game(Device& dev, const std::string& mat_file, const std::string& map_file,
       const std::string& ch_file);
  void set_focus();
  void end_turn();
  void move_up();
  void move_down();
  void move_left();
  void move_right();
};

#endif // GAME_HPP
