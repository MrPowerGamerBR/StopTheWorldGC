cmake_minimum_required(VERSION 4.2)
project(StopTheWorldGC C)

set(CMAKE_C_STANDARD 23)

add_executable(StopTheWorldGC main.c)

target_compile_options(StopTheWorldGC PRIVATE -fsanitize=address -fno-omit-frame-pointer)
target_link_options(StopTheWorldGC PRIVATE -fsanitize=address)