#include "device.hpp"

#include <cstdio>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include "game.hpp"
#include "material.hpp"
#include "inputs.hpp"

using namespace std;

const int SCREEN_DEPTH = 32;

const int TILE_SIZE = 32;

const Uint32 SDL_INIT_FLAGS = SDL_INIT_VIDEO | SDL_INIT_AUDIO;

const char* FONT = "assets/fonts/DejaVuSans.ttf";
const int FONT_SIZE = 18;
const SDL_Color FONT_COLOR = {0xcc, 0xcc, 0xcc, 0xff};

const SDL_Color MENU_COLOR = {0x1f, 0x0f, 0x45, 0xff};

const Uint8 BG_R = 0x08;
const Uint8 BG_G = 0x08;
const Uint8 BG_B = 0x08;
const Uint8 BG_A = 0xff;
const SDL_Color BG_COLOR = {BG_R, BG_G, BG_B, BG_A};

//#define PLAY_MUSIC
const char* MUSIC_FILE = "assets/audio/df_music.ogg";
const int AUDIO_FREQUENCY = 44100;
const int AUDIO_CHANNELS = 2; // stereo
const int AUDIO_BUFFER_SIZE = 4096;

SDL_Window* g_win;
SDL_Renderer* g_renderer;
TTF_Font* g_font;
Mix_Music* g_music;

Device::Device(const int screen_w, const int screen_h) {
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
  g_win = SDL_CreateWindow("Dungeon Master by David Cavazos", 500, 100,
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
    is_running &= process_input(event, game);
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
  SDL_QueryTexture(tex, NULL, NULL, &dest.w, &dest.h);
  dest.x = x;
  dest.y = y;
  SDL_RenderCopy(g_renderer, tex, NULL, &dest);
}

void Device::draw_game(const Game& g) {
  srand(42);
  SDL_Rect src, dest;
  src.w = TILE_SIZE;
  src.h = TILE_SIZE;
  dest.w = TILE_SIZE;
  dest.h = TILE_SIZE;

  // draw map
  for (size_t y = 0; y < g.map.size(); ++y) {
    for (size_t x = 0; x < g.map[y].size(); ++x) {
      size_t m = g.map[y][x];
      const material& mat = g.materials[m];
      src.x = mat.pos_x + (rand() % mat.n_x) * TILE_SIZE;
      src.y = mat.pos_y + (rand() % mat.n_y) * TILE_SIZE;
      dest.x = int(x - g.focus_x) * TILE_SIZE + _width / 2;
      dest.y = int(y - g.focus_y) * TILE_SIZE + (_height - TILE_SIZE) / 2;
      SDL_RenderCopy(g_renderer, _textures[mat.image], &src, &dest);
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
  for (size_t i = 0; i < g.characters.size(); ++i) {
    const character& ch = g.characters[i];
    SDL_Texture* tex = _textures[ch.image];
    SDL_QueryTexture(tex, NULL, NULL, &dest.w, &dest.h);
    dest.x = ch.pos.x * TILE_SIZE;
    dest.y = ch.pos.y * TILE_SIZE;
    SDL_RenderCopy(g_renderer, tex, NULL, &dest);
  }
}

void Device::render() {
  SDL_RenderPresent(g_renderer);
}
