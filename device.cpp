#include "device.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include "game.hpp"
#include "material.hpp"
#include "inputs.hpp"

using namespace std;

const int SCREEN_DEPTH = 32;

const int TILE_SIZE = 64;

const Uint32 SDL_INIT_FLAGS = SDL_INIT_VIDEO | SDL_INIT_AUDIO;

const char* FONT = "assets/fonts/DejaVuSansMono.ttf";
const int FONT_SIZE = 18;
const SDL_Color FONT_COLOR = {0xcc, 0xcc, 0xcc, 0xff};

const SDL_Color MENU_COLOR = {0x1f, 0x0f, 0x45, 0xff};

const Uint8 BG_R = 0x08;
const Uint8 BG_G = 0x08;
const Uint8 BG_B = 0x08;
const Uint8 BG_A = 0xff;
const SDL_Color BG_COLOR = {BG_R, BG_G, BG_B, BG_A};

#define PLAY_MUSIC
const char* MUSIC_FILE = "assets/audio/df_music.ogg";
const int AUDIO_FREQUENCY = 44100;
const int AUDIO_CHANNELS = 2; // stereo
const int AUDIO_BUFFER_SIZE = 4096;

SDL_Window* g_win;
SDL_Renderer* g_renderer;
TTF_Font* g_font;
Mix_Music* g_music;

const Game* g_game;
bool character_posy_comp(size_t i, size_t j) {
  return g_game->characters[i].pos.y < g_game->characters[j].pos.y;
}

Device::Device(const int screen_w, const int screen_h) {
  is_edit_mode = false;
  random_obstacles = 20;
  random_seed = time(0);
  _width = screen_w;
  _height = screen_h;

  puts("Initializing SDL");
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    fprintf(stderr, "%s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);

  puts("Initializing TrueType Fonts");
  if (TTF_Init() == -1) {
    fprintf(stderr, "%s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  g_font = TTF_OpenFont(FONT, FONT_SIZE);
  if (g_font == NULL) {
    fprintf(stderr, "%s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  puts("Initializing audio");
  if (Mix_OpenAudio(AUDIO_FREQUENCY, MIX_DEFAULT_FORMAT, AUDIO_CHANNELS,
                AUDIO_BUFFER_SIZE) != 0) {
    fprintf(stderr, "%s\n", SDL_GetError());
  }
  printf("Loading audio: %s\n", MUSIC_FILE);
  g_music = Mix_LoadMUS(MUSIC_FILE);
  if (g_music == NULL) {
    fprintf(stderr, "%s (%s)\n", SDL_GetError(), MUSIC_FILE);
  } else {
#ifdef PLAY_MUSIC
    Mix_PlayMusic(g_music, -1);
#endif
  }

  puts("Creating window");
  g_win = SDL_CreateWindow("Dungeon Master by David Cavazos", 300, 20,
                          _width, _height, SDL_WINDOW_SHOWN);
  if (g_win == NULL) {
    fprintf(stderr, "%s\n", SDL_GetError());
    SDL_Quit();
    exit(EXIT_FAILURE);
  }

  puts("Creating renderer");
  g_renderer = SDL_CreateRenderer(g_win, -1,
                         SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (g_renderer == NULL) {
    fprintf(stderr, "%s\n", SDL_GetError());
    SDL_Quit();
    exit(EXIT_FAILURE);
  }
}

Device::~Device() {
  puts("Destroying textures");
  for (size_t i = 0; i < _textures.size(); ++i) {
    SDL_DestroyTexture(_textures[i]);
  }
  puts("Destroying renderer");
  SDL_DestroyRenderer(g_renderer);
  puts("Destroying window");
  SDL_DestroyWindow(g_win);

  puts("Closing audio");
  Mix_HaltMusic();
  Mix_FreeMusic(g_music);
  Mix_CloseAudio();

  puts("Quitting TTF");
  TTF_CloseFont(g_font);
  TTF_Quit();
  puts("Quitting SDL");
  SDL_Quit();
}

size_t Device::get_time() {
  return SDL_GetTicks();
}

void Device::set_title(const char* title) {
  SDL_SetWindowTitle(g_win, title);
}

void Device::sleep(unsigned int ms) {
  SDL_Delay(ms);
}

size_t Device::load_image(const string& filename) {
  size_t idx = 0;
  map<string,size_t>::iterator it = _textures_idx.find(filename);
  if (it != _textures_idx.end()) {
    idx = it->second;
  } else {
    printf("Loading image: %s\n", filename.c_str());
    SDL_Texture* tex = IMG_LoadTexture(g_renderer, filename.c_str());
    if (tex == NULL) {
      fprintf(stderr, "%s (%s)\n", SDL_GetError(), filename.c_str());
    }
    idx = _textures.size();
    _textures_idx.insert(make_pair(filename, idx));
    _textures.push_back(tex);
  }
  return idx;
}

bool Device::process_events(Game& game) {
  bool is_running = true;
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    is_running &= process_input(event, *this, game);
  }
  return is_running;
}

void Device::clear_screen() {
  SDL_SetRenderDrawColor(g_renderer, BG_R, BG_G, BG_B, BG_A);
  SDL_RenderClear(g_renderer);
}

void Device::draw_line(int x1, int y1, int x2, int y2, const SDL_Color& c) {
  SDL_SetRenderDrawColor(g_renderer, c.r, c.g, c.b, c.a);
  SDL_RenderDrawLine(g_renderer, x1, y1, x2, y2);
}

void Device::draw_rect(int x, int y, int w, int h, const SDL_Color& c) {
  SDL_SetRenderDrawColor(g_renderer, c.r, c.g, c.b, c.a);
  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;
  SDL_RenderDrawRect(g_renderer, &rect);
}

void Device::draw_fill_rect(int x, int y, int w, int h, const SDL_Color& c) {
  SDL_SetRenderDrawColor(g_renderer, c.r, c.g, c.b, c.a);
  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;
  SDL_RenderFillRect(g_renderer, &rect);
}

void Device::draw_text(int x, int y, const string& text) {
  size_t idx = 0;
  SDL_Texture* tex;
  map<string,size_t>::iterator it = _texts_idx.find(text);
  if (it != _texts_idx.end()) {
    idx = it->second;
    tex = _textures[idx];
  } else {
    SDL_Surface* s = TTF_RenderText_Blended(g_font, text.c_str(), FONT_COLOR);
    tex = SDL_CreateTextureFromSurface(g_renderer, s);
    SDL_FreeSurface(s);
    idx = _textures.size();
    _texts_idx.insert(make_pair(text, idx));
    _textures.push_back(tex);
  }
  SDL_Rect dest;
  dest.x = x;
  dest.y = y;
  SDL_QueryTexture(tex, NULL, NULL, &dest.w, &dest.h);
  SDL_RenderCopy(g_renderer, tex, NULL, &dest);
}

void Device::draw_game(const Game& g) {
  g_game = &g;

  srand(42);
  SDL_Rect src, dest;
  src.w = TILE_SIZE;
  src.h = TILE_SIZE;
  dest.w = TILE_SIZE;
  dest.h = TILE_SIZE;
  const int SQR = TILE_SIZE - 1;

  // draw map
  const character& ch1 = g.characters[g.turns[0]];
  for (size_t y = 0; y < g.map.size(); ++y) {
    for (size_t x = 0; x < g.map[0].size(); ++x) {
      size_t m = g.map[y][x];
      const material& mat = g.materials[m];
      src.x = mat.pos_x + (rand() % mat.n_x) * TILE_SIZE;
      src.y = mat.pos_y + (rand() % mat.n_y) * TILE_SIZE;
      dest.x = pos_x(g, x);
      dest.y = pos_y(g, y);
      SDL_RenderCopy(g_renderer, _textures[mat.image], &src, &dest);

      // draw attack range
      if (is_edit_mode) {
        continue;
      }
      double dist_x = abs(double(ch1.pos.x) - x);
      double dist_y = abs(double(ch1.pos.y) - y);
      int dist = pow(dist_x * dist_x + dist_y * dist_y, 0.5) + 0.5;
      if (g.materials[g.map[y][x]].is_walkable && dist <= ch1.range) {
        draw_rect(dest.x, dest.y, SQR, SQR, {255,255,0,255});
      }
    }
  }

  // draw grid
  for (int i = TILE_SIZE - 1; i < _height; i += TILE_SIZE) {
    draw_line(0, i, _width, i, BG_COLOR);
  }
  for (int i = TILE_SIZE - 1; i < _width; i += TILE_SIZE) {
    draw_line(i, 0, i, _height, BG_COLOR);
  }

  // draw characters
  SDL_Color color;
  vector<size_t> sorted(g.characters.size());
  for (size_t i = 0; i < sorted.size(); ++i) {
    sorted[i] = i;
  }
  sort(sorted.begin(), sorted.end(), character_posy_comp);
  for (size_t i = 0; i < sorted.size(); ++i) {
    const character& ch = g.characters[sorted[i]];
    if (ch.is_playable) {
      color = {0,255,255,255};
    } else {
      color = {255,0,0,255};
    }
    SDL_Texture* tex = _textures[ch.image];
    SDL_QueryTexture(tex, NULL, NULL, &dest.w, &dest.h);
    dest.x = pos_x(g, ch.pos.x);
    dest.y = pos_y(g, ch.pos.y);
    draw_rect(dest.x, dest.y, (TILE_SIZE - 1) * ch.base_size,
              (TILE_SIZE - 1) * ch.base_size, color);
    dest.x = dest.x - ch.base_start;
    dest.y = dest.y - dest.h + TILE_SIZE - 2;
    SDL_RenderCopy(g_renderer, tex, NULL, &dest);

    // draw HP bar
    const int BARH = 7; // health bar height
    dest.x = pos_x(g, ch.pos.x) + 1;
    dest.y = pos_y(g, ch.pos.y) + TILE_SIZE + 1;
    int bar_len = TILE_SIZE - 3;
    float perc = double(ch.hp) / ch.hp_max;
    draw_rect(dest.x, dest.y, bar_len, BARH, {0,0,0,255});

    ++dest.x;
    ++dest.y;
    bar_len = perc * (TILE_SIZE - 5);
    if (perc > 1.0f / 2.0f) {
      color = {0,255,0,255};
    } else {
      color = {255,255,0,255};
    }
    draw_fill_rect(dest.x, dest.y, bar_len, BARH - 2, color);
    draw_fill_rect(dest.x + bar_len, dest.y, TILE_SIZE - bar_len - 5, BARH - 2,
                   {255,0,0,255});
  }

  // draw info
  draw_text(10, 45, "[ESC]      Exit");
  draw_text(10, 65, "[TAB]      Toggle Edit Mode");
  if (is_edit_mode) {
    draw_text(10,   5, "Mode: Edit");
    draw_text(10,  25, "[W,A,S,D]  Move");

    draw_text(10,  85, "[1]        Grass");
    draw_text(10, 105, "[2]        Dirt");
    draw_text(10, 125, "[3]        Stone");
    draw_text(10, 145, "[4]        Wall");
    draw_text(10, 165, "[R]        Randomize (" + to_string(random_obstacles) +
              "% obstacles)");
    draw_text(10, 185, "[J]        Increase obstacles %");
    draw_text(10, 205, "[K]        Decrease obstacles %");
    draw_text(10, 225, "[T]        Randomize seed");
    draw_text(10, 245, "[M]        Read map from file");
    draw_text(10, 265, "[0]        Place Kibus");
    draw_text(10, 285, "[9]        Toggle Ghast");
  } else {
    const character& ch = g.characters[g.turns[0]];
    draw_text(400,  5, ch.name);
    draw_text(400, 25, "HP: " + to_string(ch.hp) + "/" + to_string(ch.hp_max));
    draw_text(10,   5, "Mode: Battle");
    draw_text(10,  25, "[W,A,S,D,Q,E,Z,C]  Moves: " + to_string(g.move_limit));

    draw_text(10,  85, "[SPC]      End Turn");
    draw_text(10, 105, "[RET]      Attack");
  }

  // edit mode
  if (is_edit_mode) {
    draw_rect(pos_x(g, g.focus_x), pos_y(g, g.focus_y), TILE_SIZE - 1,
              TILE_SIZE - 1, {255,255,0,255});
  }
}

void Device::render() {
  SDL_RenderPresent(g_renderer);
}

void Device::randomize_map(Game& g) {
  float total = g.map[0].size() * g.map.size();
  int obstacles = total * random_obstacles * 0.01;
  size_t x, y;

  srand(clock());
  // clear map
  for (y = 0; y < g.map.size(); ++y) {
    for (x = 0; x < g.map[0].size(); ++x) {
      int temp = rand() % 10;
      if (temp < 5) {
        g.map[y][x] = 0;
      } else if (temp < 9) {
        g.map[y][x] = 1;
      } else {
        g.map[y][x] = 2;
      }
    }
  }

  // place obstacles
  srand(random_seed);
  for (int i = 0; i < obstacles; ++i) {
    do {
      x = rand() % g.map[0].size();
      y = rand() % g.map.size();
    } while (g.map[y][x] == 3);
    g.map[y][x] = 3;
  }
}



int Device::pos_x(const Game& g, int x) {
  return (x - g.focus_x) * TILE_SIZE + _width / 2;
}

int Device::pos_y(const Game& g, int y) {
  return (y - g.focus_y) * TILE_SIZE + _height / 2;
}
