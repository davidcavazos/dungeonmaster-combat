#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <map>
#include <string>
#include <vector>
#include <SDL2/SDL.h>

class Game;

class Device {
public:
  bool is_edit_mode;
  int random_obstacles;
  int random_seed;

  Device(const int screen_w, const int screen_h);
  ~Device();

  size_t get_time();
  void set_title(const char* title);
  void sleep(unsigned int ms);
  size_t load_image(const std::string& filename);
  bool process_events(Game& game);
  void clear_screen();
  void draw_line(int x1, int y1, int x2, int y2, const SDL_Color& c);
  void draw_rect(int x, int y, int w, int h, const SDL_Color& c);
  void draw_fill_rect(int x, int y, int w, int h, const SDL_Color& c);
  void draw_text(int x, int y, const std::string& text);
  void draw_game(const Game& g);
  void render();
  void randomize_map(Game& g);
  int pos_x(const Game& g, int x);
  int pos_y(const Game& g, int y);

private:
  int _width;
  int _height;
  std::vector<SDL_Texture*> _textures;
  std::map<std::string,size_t> _textures_idx;
  std::map<std::string,size_t> _texts_idx;
};

#endif // DEVICE_HPP
