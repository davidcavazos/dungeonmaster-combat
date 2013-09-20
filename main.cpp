#include <cstdio>
#include <cstdlib>
#include <string>
#include "config.hpp"
#include "device.hpp"
#include "game.hpp"

using namespace std;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int FRAME_CAP = 60;
const int FRAME_CAP_MS = 1000 / FRAME_CAP;

const string MATERIALS_FILENAME = "assets/materials";
const string MAP_FILENAME = "assets/map1";
const string CHARACTERS_FILENAME = "assets/characters";

int main(int, char**) {
  printf("Dungeon Master v%d.%d\n", VERSION_MAJOR, VERSION_MINOR);
  puts("Designed and programmed by: David Cavazos");
  puts("Music from: Dwarf Fortress");

  Device dev(SCREEN_WIDTH, SCREEN_HEIGHT);
  Game game(dev, MATERIALS_FILENAME, MAP_FILENAME, CHARACTERS_FILENAME);

  puts("Running game loop");
  char buffer[64];
  unsigned int start_time;
  unsigned int delta_time;
  bool is_running = true;
  while (is_running) {
    start_time = dev.get_time();
    dev.clear_screen();

    is_running = dev.process_events(game);

    // draw to screen
    dev.draw_game(game);

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
