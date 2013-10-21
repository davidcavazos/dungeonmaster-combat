#include <cstdio>
#include <cstdlib>
#include <string>
#include "config.hpp"
#include "device.hpp"
#include "game.hpp"
#include "ai.hpp"

using namespace std;

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;

const int FRAME_CAP = 60;
const int FRAME_CAP_MS = 1000 / FRAME_CAP;

const int AI_UPDATE = 10; // in frames

const string MATERIALS_FILENAME = "assets/materials";
const string MAP_FILENAME = "assets/map2";
const string CHARACTERS_FILENAME = "assets/characters";
const string ENEMIES_FILENAME = "assets/enemies";

int main(int, char**) {
  printf("Dungeon Master v%d.%d\n", VERSION_MAJOR, VERSION_MINOR);
  puts("Designed and programmed by: David Cavazos");
  puts("Music from: Dwarf Fortress");

  Device dev(SCREEN_WIDTH, SCREEN_HEIGHT);
  Game g(dev, MATERIALS_FILENAME, MAP_FILENAME, CHARACTERS_FILENAME,
         ENEMIES_FILENAME);

  puts("Running game loop");
  char buffer[64];
  unsigned int start_time;
  unsigned int delta_time;
  bool is_running = true;
  while (is_running) {
    start_time = dev.get_time();

    // game logic
    is_running = dev.process_events(g);
    if (!dev.is_edit_mode) {
      process_ai(g);
    }

    // draw to screen
    dev.clear_screen();
    dev.draw_game(g);
    draw_ai(dev, g);

    // control framerate
    delta_time = dev.get_time() - start_time;
    sprintf(buffer, "Dungeon Master by David Cavazos - %02u ms", delta_time);
    dev.set_title(buffer);

    dev.render();
    if (FRAME_CAP_MS > delta_time) {
      dev.sleep(FRAME_CAP_MS - delta_time);
    }
  }
  return EXIT_SUCCESS;
}
