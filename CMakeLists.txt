cmake_minimum_required(VERSION 3.15)
project(solitaire)

add_subdirectory(libs/raylib EXCLUDE_FROM_ALL)
add_subdirectory(libs/raylib-nuklear EXCLUDE_FROM_ALL)

set(CMAKE_C_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/libs/tomlc99)
include_directories(${CMAKE_SOURCE_DIR}/libs/raylib-nuklear/include)

add_executable(${PROJECT_NAME} 
  src/gfx/animated.c
  src/gfx/cards.c
  src/gfx/layout.c
  src/io/config.c
  src/io/pacman.c
  src/io/spritesheet.c
  src/io/texturepack.c
  src/io/textures.c
  src/scenes/game.c 
  src/scenes/menu.c 
  src/scenes/scene.c
  src/scenes/settings.c
  src/sfx/audio.c
  src/solitaire.c 
  src/main.c 
  src/util.c
  
  libs/tomlc99/toml.c
)

target_link_libraries(${PROJECT_NAME} raylib raylib_nuklear)

if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()