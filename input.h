#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>
#include <vector>
#include "physics.h"
#include "audio.h"

extern double xpos, ypos;
extern bool isMouseHeld;
extern bool isSpacePressed;
extern int selectedObject;
extern std::vector<Module> Modules;
extern AudioData audioData; // Add the audioData declaration

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

#endif // INPUT_H

