cmake_minimum_required(VERSION 3.15)
project(solitaire)

add_subdirectory(libs/raylib EXCLUDE_FROM_ALL)

set(CMAKE_C_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/libs/tomlc99)

add_executable(${PROJECT_NAME} 
  src/scenes/menu.c 
  src/solitaire.c 
  src/main.c 
  src/scene.c 
  
  libs/tomlc99/toml.c
)

target_link_libraries(${PROJECT_NAME} raylib)

if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()