cmake_minimum_required(VERSION 3.15)
project(starfield)

find_package(SDL3 REQUIRED CONFIG COMPONENTS SDL3-static)

add_executable(starfield WIN32 main.c)
target_link_libraries(starfield PRIVATE SDL3::SDL3-static)