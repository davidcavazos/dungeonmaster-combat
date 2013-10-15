cmake_minimum_required (VERSION 2.6)
project (dungeonmaster)

set (VERSION_MAJOR 0)
set (VERSION_MINOR 2)
configure_file (
  "${PROJECT_SOURCE_DIR}/config.hpp.in"
  "${PROJECT_SOURCE_DIR}/config.hpp"
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

if (NOT SDL2_FOUND)
  message (FATAL_ERROR "SDL2 required and not found")
endif(NOT SDL2_FOUND)
if (NOT SDL2_IMAGE_FOUND)
  message (FATAL_ERROR "SDL2 image required and not found")
endif(NOT SDL2_IMAGE_FOUND)
if (NOT SDL2_TTF_FOUND)
  message (FATAL_ERROR "SDL2 ttf required and not found")
endif(NOT SDL2_TTF_FOUND)

link_libraries(
  ${SDL2_LIBRARY}
  ${SDL2_IMAGE_LIBRARY}
  ${SDL2_MIXER_LIBRARY}
  ${SDL2_TTF_LIBRARY}
  SDL2main
)

add_definitions(-std=c++11 -Wall -Wextra -Wfatal-errors -pedantic -O3)

set(SRC
  main.cpp
  device.cpp
  game.cpp
  ai.cpp
)

add_executable(dungeonmaster WIN32 MACOSX_BUNDLE ${SRC})
