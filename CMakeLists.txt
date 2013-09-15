cmake_minimum_required (VERSION 2.6)
project (dungeonmaster)

set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)
configure_file (
  "${PROJECT_SOURCE_DIR}/config.hpp.in"
  "${PROJECT_SOURCE_DIR}/config.hpp"
)

find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)

if (NOT SDL_FOUND)
  message (FATAL_ERROR "SDL required and not found")
endif(NOT SDL_FOUND)

link_libraries(
  ${SDL_LIBRARY}
  ${SDLIMAGE_LIBRARY}
  SDLmain
)

add_definitions(-std=c++11 -Wall -Wextra -Wfatal-errors -pedantic -O3)

set(SRC
  main.cpp
)

add_executable(dungeonmaster WIN32 MACOSX_BUNDLE ${SRC})
