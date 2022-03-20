cmake_minimum_required(VERSION 3.21)
project(compilerbau_pkk C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(compilerbau_pkk
        src/main.c
        src/turtle-eval.c
        src/turtle-nametab.c
        src/sdlinterf.c
        src/turtle-types.h
        src/turtle.h
        src/sdlinterf.h
        src/lexer.c src/dynamic_array.c src/dynamic_array.h)
INCLUDE(FindPkgConfig)

PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)

INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES} m)