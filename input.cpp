#include "input.h"
#include "physics.h"
#include "audio.h"
#include <GLFW/glfw3.h>
#include <iostream>

extern AudioData audioData; // Ensure this is declared to use in the callback
bool isOPressed = false;

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    int windowWidth, windowHeight;
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

    // Convert screen coordinates to world coordinates
    ::xpos = xpos * 2;
    ::ypos = (ypos * 2) - 0.5 * windowHeight;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        isMouseHeld = true;
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        isMouseHeld = false;
        selectedObject = -1;
    }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        isSpacePressed = true;
        instantiate(xpos, ypos, &audioData); // Pass the audioData pointer
        std::cout << "space" << std::endl;
    }
    else if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
        isOPressed = true;
        instantiateO(xpos, ypos, &audioData);
        std::cout << "oscillator" << std::endl;
    }
    // else if (key == GLFW_KEY_S && action == GLFW_PRESS)
    // {
    //     isSPressed = true;
    //     instantiateS(xpos, ypos, &audioData);
    //     std::cout << "sequencer" << std::endl;
    // }
    else
    {
        isSpacePressed = false;
        isOPressed = false;
    }
}
