#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SDL2/SDL.h>
#include "audio.h"
#include "physics.h"
#include "input.h"
#include "render.h"
#include "constants.h"

// Declare audioData globally
AudioData audioData = { 440.0, 28000, SDL_CreateMutex() };

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

    startAudioStream(&audioData);

    instantiate(100.0, 100.0, &audioData); // Correctly pass the audioData pointer

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
        physicsProcess(Modules, &audioData, dt); // Correctly pass the audioData pointer and dt
        render(Modules, windowWidth, windowHeight, squareSize);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // Example of updating the frequency dynamically
        for (auto& module : Modules)
        {
            module.in += 1.0f;
            if (module.in > 880.0f)
                module.in = 440.0f;
            module.out = module.in; // For this example, directly map input to output
            module.updateFrequency(&audioData);
        }

        SDL_Delay(100); // Add delay to slow down frequency updates
    }

    SDL_CloseAudio();
    SDL_DestroyMutex(audioData.mutex);
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

