cmake_minimum_required(VERSION 3.21)
project(compilerbau_pkk C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(compilerbau_pkk
        main.c
        turtle-eval.c
        turtle-nametab.c
        turtle-types.h
        turtle.h)
