#include "render.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

int windowWidth = 800;
int windowHeight = 600;

void render(const std::vector<Module>& Modules, int windowWidth, int windowHeight, int squareSize)
{
    for (const Module& obj : Modules)
    {
        int x = obj.position_current.x;
        int y = obj.position_current.y;

        float centerX = x - squareSize / 2.0f;
        float centerY = y - squareSize / 2.0f;

        float left = 2.0f * centerX / windowWidth - 1.0f;
        float right = 2.0f * (centerX + squareSize) / windowWidth - 1.0f;
        float top = 1.0f - 2.0f * centerY / windowHeight;
        float bottom = 1.0f - 2.0f * (centerY + squareSize) / windowHeight;

        glBegin(GL_POLYGON);
        glVertex2f(left, top);
        glVertex2f(right, top);
        glVertex2f(right, bottom);
        glVertex2f(left, bottom);
        glEnd();
    }
}

