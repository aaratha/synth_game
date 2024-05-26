cmake_minimum_required(VERSION 3.10)

project(engine)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the build type
set(CMAKE_BUILD_TYPE Debug)

# Include directories
include_directories(/opt/homebrew/include)

# Find packages
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SFML 2.5 COMPONENTS audio REQUIRED)
find_package(GLFW3 REQUIRED)

# Add the executable
add_executable(engine main.cpp audio.cpp input.cpp render.cpp oscillator.cpp physics.cpp)

# Link libraries
target_link_libraries(engine PRIVATE ${OPENGL_LIBRARIES} GLEW::GLEW SDL2::SDL2 SDL2::SDL2main sfml-audio glfw)
