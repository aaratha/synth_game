#ifndef PHYSICS_H
#define PHYSICS_H

#include <SFML/Graphics.hpp>
#include <vector>

using Vec2 = sf::Vector2f;

Vec2 normalize(const Vec2& vector);

struct physicsObject
{
    Vec2 velocity;
    Vec2 position_current;
    Vec2 position_old;
    Vec2 acceleration;

    void updatePosition(float dt);
    void accelerate(Vec2 acc);
};

struct Module : public physicsObject
{
    int frequency;
    int amplitude;
    double phase;

    float in;
    float out;

    float modifier;

    Module(int freq, int amp);
    void generateSound(double dt);
};

extern std::vector<Module> Modules;
extern double xpos, ypos;
extern bool isMouseHeld;
extern int selectedObject;

void instantiate(double x, double y);
void updatePositions(std::vector<Module>& Modules, float dt);
void applyDrag(std::vector<Module>& Modules);
void applyDamping(std::vector<Module>& Modules);
void solveCollisions(std::vector<Module>& Modules);
void physicsProcess(std::vector<Module>& Modules, float dt);

#endif // PHYSICS_H

