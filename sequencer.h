#ifndef SEQUENCER_H
#define SEQUENCER_H

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
    void updateFrequency(AudioData* audioData, float elapsedTime);
};

#endif // OSCILLATOR_H
