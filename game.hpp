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
  int diag_moves;
  std::vector<material> materials;
  std::vector<std::vector<size_t> > map;
  std::vector<character> characters;
  std::vector<size_t> turns;

  Game(Device& dev, const std::string& mat_file, const std::string& map_file,
       const std::string& ch_file);
  void load_map(const std::string& file);
  void set_focus();
  void end_turn();
  void move_up();
  void move_down();
  void move_left();
  void move_right();
  void move_left_up();
  void move_right_up();
  void move_left_down();
  void move_right_down();
};

#endif // GAME_HPP
