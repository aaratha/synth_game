#include "input.h"
#include "physics.h"
#include <iostream>

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    ::xpos = xpos;
    ::ypos = ypos;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        isMouseHeld = true;
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        isMouseHeld = false;
        selectedObject = -1;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        isSpacePressed = true;
        instantiate(xpos, ypos);
        std::cout << "space" << std::endl;
    }
    else
        isSpacePressed = false;
}

