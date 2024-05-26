#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SDL2/SDL.h>
#include "audio.h"
#include "physics.h"
#include "input.h"
#include "render.h"
#include "constants.h"
#include "oscillator.h"

// mac compile command:
//     c++ -std=c++11 -o engine main.cpp audio.cpp render.cpp input.cpp oscillator.cpp -I/opt/homebrew/include -L/opt/homebrew/lib -lGL -lGLEW -lSDL2 -lsfml-graphics -lGLFW -lGLM

// Declare audioData globally
AudioData audioData = {440.0, 28000, SDL_CreateMutex()};

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

int main()
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // #ifdef __APPLE__
    //     glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    //     glfwWindowHint(GLFW_OPENGL_COMPAT_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // #endif

    GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight, "GLFW OpenGL Game", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    startAudioStream(&audioData);

    glViewport(0, 0, windowWidth, windowHeight);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);

    initRender();

    float elapsedTime = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float dt = 0.016f;
        elapsedTime += dt;

        physicsProcess(Modules, &audioData, dt); // Correctly pass the audioData pointer and dt
        render(Modules, windowWidth, windowHeight, squareSize);

        // Use LFO to modulate the frequency
        for (auto &module : Modules)
        {
            module.updateFrequency(&audioData, elapsedTime);
            module.generateSound(dt); // Ensure sound generation uses updated frequency
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    SDL_CloseAudio();
    SDL_DestroyMutex(audioData.mutex);
    SDL_Quit();
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}
