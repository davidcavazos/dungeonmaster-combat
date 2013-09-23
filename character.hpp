#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <string>

typedef struct {
  int strength;
  int dexterity;
  int constitution;
  int intelligence;
  int wisdom;
  int charisma;
} statistics;

typedef struct {
  int x;
  int y;
} position;

typedef struct {
  std::string name;
  size_t image;
  bool is_playable;
  int base_start;
  int base_size;
  statistics stats;
  position pos;
  int move_limit;
} character;

#endif // CHARACTER_HPP
