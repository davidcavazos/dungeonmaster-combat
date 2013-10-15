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
  int hp;
  int hp_max;
  int armor_class;
  statistics stats;
  position pos;
  int move_limit;
  int attack_bonus;
  int critical;
  int range;
  int ammo;
  int damage;
} character;

#endif // CHARACTER_HPP
