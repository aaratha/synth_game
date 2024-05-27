#ifndef PHYSICS_H
#define PHYSICS_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "audio.h"

using Vec2 = sf::Vector2f;

Vec2 normalize(const Vec2 &vector);

struct physicsObject
{
    Vec2 position;

    void applyDrag();
};

struct Module : public physicsObject
{
    int amplitude;
    double phase;

    float in;
    float out;

    float modifier;

    Module(int freq, int amp, AudioData *audioData);

    void generateSound(double dt);
    virtual void updateFrequency(AudioData *audioData, float elapsedTime); // Mark as virtual
};

extern std::vector<Module> Modules;
extern double xpos, ypos;
extern bool isMouseHeld;
extern int selectedObject;

void instantiate(double x, double y, AudioData *audioData);
void instantiateO(double x, double y, AudioData *audioData);
void updatePositions(std::vector<Module> &Modules, float dt);
void physicsProcess(std::vector<Module> &Modules, AudioData *audioData, float dt);

#endif // PHYSICS_H
