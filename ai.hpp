#ifndef AI_HPP
#define AI_HPP

#include <cstddef>

class Device;
class Game;

void create_character_ai(Game& g, size_t idx);
void delete_character_ai(Game& g, size_t idx);
void process_ai(Game& g);
void draw_ai(Device& dev, Game& g);

#endif // AI_HPP
