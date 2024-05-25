#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include <SFML/Graphics.hpp>
#include "audio.h"
#include "physics.h"

using Vec2 = sf::Vector2f;

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

#endif // OSCILLATOR_H

