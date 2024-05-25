#ifndef PHYSICS_H
#define PHYSICS_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "audio.h"

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
    int amplitude;
    double phase;

    float in;
    float out;

    float modifier;

    Module(int freq, int amp, AudioData* audioData);

    void generateSound(double dt);
    void updateFrequency(AudioData* audioData);
};

extern std::vector<Module> Modules;
extern double xpos, ypos;
extern bool isMouseHeld;
extern int selectedObject;

void instantiate(double x, double y, AudioData* audioData);
void updatePositions(std::vector<Module>& Modules, float dt);
void applyDrag(std::vector<Module>& Modules);
void applyDamping(std::vector<Module>& Modules);
void solveCollisions(std::vector<Module>& Modules);
void physicsProcess(std::vector<Module>& Modules, AudioData* audioData, float dt);
void inToOut(std::vector<Module>& Modules, AudioData* audioData);

#endif // PHYSICS_H

