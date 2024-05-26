#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include <SFML/Graphics.hpp>
#include "audio.h"
#include "physics.h"

using Vec2 = sf::Vector2f;

struct Oscillator : public Module
{
    Oscillator(int freq, int amp, AudioData* audioData) : Module(freq, amp, audioData) {}

    void updateFrequency(AudioData* audioData, float elapsedTime) override; // Ensure override is used
};

#endif // OSCILLATOR_H

