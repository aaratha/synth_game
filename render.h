#ifndef RENDER_H
#define RENDER_H

#include <vector>
#include "physics.h"
#include <GL/glew.h>

extern int windowWidth;
extern int windowHeight;

void initRender();
void render(const std::vector<Module>& Modules, int windowWidth, int windowHeight, int squareSize);

#endif // RENDER_H
