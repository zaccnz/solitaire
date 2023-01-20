cmake_minimum_required(VERSION 3.15)
project(solitaire)

add_subdirectory(libs/physfs EXCLUDE_FROM_ALL)
add_subdirectory(libs/raylib EXCLUDE_FROM_ALL)
add_subdirectory(libs/raylib-nuklear EXCLUDE_FROM_ALL)

set(CMAKE_C_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/libs/tomlc99)
include_directories(${CMAKE_SOURCE_DIR}/libs/raylib-nuklear/include)

add_executable(${PROJECT_NAME} 
  libs/tomlc99/toml.c

  src/gfx/animations.c
  src/gfx/animator.c
  src/gfx/cards.c
  src/gfx/layout.c
  src/io/tex/spritesheet.c
  src/io/tex/texturepack.c
  src/io/tex/textures.c
  src/io/config.c
  src/io/leaderboard.c
  src/io/licences.c
  src/io/pacman.c
  src/scenes/game.c
  src/scenes/leaderboard.c
  src/scenes/menu.c
  src/scenes/scene.c
  src/scenes/settings.c
  src/sfx/audio.c
  src/util/toml_writer.c
  src/util/unitbezier.c
  src/util/util.c
  src/solitaire.c
  src/main.c
)

target_link_libraries(${PROJECT_NAME} raylib raylib_nuklear physfs-static)

if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()