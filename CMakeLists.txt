cmake_minimum_required(VERSION 3.15)
project(solitaire)

set(PHYSFS_ARCHIVE_ZIP ON CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_7Z OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_GRP OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_WAD OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_HOG OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_MVL OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_QPAK OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_SLB OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_ISO9660 OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_VDF OFF CACHE BOOL "" FORCE)

set(PHYSFS_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(PHYSFS_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(PHYSFS_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(PHYSFS_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(PHYSFS_DISABLE_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(libs/physfs EXCLUDE_FROM_ALL)
add_subdirectory(libs/raylib EXCLUDE_FROM_ALL)
add_subdirectory(libs/raylib-nuklear EXCLUDE_FROM_ALL)

set(CMAKE_C_STANDARD 99)

if (EMSCRIPTEN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -sEXPORTED_RUNTIME_METHODS=ccall")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -sASYNCIFY -s \"ASYNCIFY_IMPORTS=['emscripten_asm_const_int']\" -O3")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s EXPORTED_FUNCTIONS='[\"_resize_event\", \"_main\", \"_malloc\", \"_free\"]'")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s ALLOW_MEMORY_GROWTH=1 -s TOTAL_STACK=8MB -s INITIAL_MEMORY=16MB")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -o index.html --shell-file ${CMAKE_SOURCE_DIR}/template.html")
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif ()

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/libs/tomlc99)
include_directories(${CMAKE_SOURCE_DIR}/libs/raylib-nuklear/include)

add_executable(${PROJECT_NAME} 
  libs/tomlc99/toml.c

  src/gfx/animations.c
  src/gfx/animator.c
  src/gfx/background.c
  src/gfx/cards.c
  src/gfx/layout.c
  src/io/tex/assets.c
  src/io/tex/spritesheet.c
  src/io/tex/texturepack.c
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
  src/util/debug.c
  src/util/emscripten.c
  src/util/toml_writer.c
  src/util/unitbezier.c
  src/util/util.c
  src/solitaire.c
  src/main.c
)

target_link_libraries(${PROJECT_NAME} raylib raylib_nuklear physfs-static)

if (EMSCRIPTEN)
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-lopenal -lidbfs.js --preload-file ${CMAKE_SOURCE_DIR}/res@/res --preload-file ${CMAKE_SOURCE_DIR}/packs@/packs")
endif ()

if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()