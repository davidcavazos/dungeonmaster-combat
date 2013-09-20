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
  size_t base_start;
  size_t base_size;
  statistics stats;
  position pos;
} character;

#endif // CHARACTER_HPP
