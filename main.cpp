#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SDL2/SDL.h>
#include "audio.h"
#include "physics.h"
#include "input.h"
#include "render.h"
#include "constants.h"

// build command:
//     c++ engine main.cpp audio.cpp input.cpp render.cpp physics.cpp -lGLEW -lglfw -lSDL2 -lSFML -lGL

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "GLFW OpenGL Game", nullptr, nullptr);
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

    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_AudioSpec desiredSpec;
    SDL_AudioSpec obtainedSpec;

    SDL_memset(&desiredSpec, 0, sizeof(desiredSpec));
    desiredSpec.freq = SAMPLE_RATE;
    desiredSpec.format = AUDIO_S16SYS;
    desiredSpec.channels = 1;
    desiredSpec.samples = 4096;
    desiredSpec.callback = audio_callback;

    if (SDL_OpenAudio(&desiredSpec, &obtainedSpec) < 0)
    {
        std::cerr << "Failed to open audio: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_PauseAudio(0);
    glViewport(0, 0, windowWidth, windowHeight);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);

    initRender();

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float dt = 0.016f;
        physicsProcess(Modules, dt);
        render(Modules, windowWidth, windowHeight, squareSize);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    SDL_CloseAudio();
    SDL_Quit();
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

