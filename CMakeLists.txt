cmake_minimum_required(VERSION 3.10)

project(engine)

find_package(sdl2 CONFIG REQUIRED)

add_executable(engine main.cpp audio.cpp input.cpp renderer.cpp)

target_link_libraries(engine PRIVATE SDL2::SDL2main SDL2::SDL2-static)