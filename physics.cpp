#include "physics.h"
#include "oscillator.h"
#include "audio.h"
#include "constants.h"
#include "render.h"
#include <cmath>
#include <vector>
#include <iostream>

double xpos, ypos;
int deadZone = 30;
bool isMouseHeld = false;
bool isSpacePressed = false;
std::vector<Module> Modules;
int selectedObject = -1;

float elapsedTime = 0.0f;

Module::Module(int freq, int amp, AudioData *audioData)
    : amplitude(amp), phase(0.0), in(freq), out(freq), modifier(1.0f)
{
    updateFrequency(audioData, elapsedTime);
}

void Module::generateSound(double dt)
{
    double frequency = out; // Use the out parameter as the frequency
    double phaseIncrement = 2.0 * M_PI * frequency / SAMPLE_RATE;
    phase += phaseIncrement * dt;
    if (phase >= 2.0 * M_PI)
        phase -= 2.0 * M_PI;
}

void Module::updateFrequency(AudioData *audioData, float elapsedTime)
{
    // Base implementation, can be empty
}

Vec2 normalize(const Vec2 &vector)
{
    float length = std::sqrt(vector.x * vector.x + vector.y * vector.y);
    if (length != 0)
    {
        return Vec2(vector.x / length, vector.y / length);
    }
    return vector;
}

void instantiate(double x, double y, AudioData *audioData)
{
    Module obj(440, 28000, audioData);
    obj.position = Vec2(x, y);
    obj.phase = 0.0;
    obj.in = 440.0f; // Example input frequency

    std::cout << "Instantiating module at position (" << x << ", " << y << ")" << std::endl;
    Modules.push_back(obj);
}

void instantiateO(double x, double y, AudioData *audioData)
{
    Oscillator obj(440, 28000, audioData); // Instantiate Oscillator
    obj.position = Vec2(x, y);
    obj.phase = 0.0;
    obj.in = 440.0f; // Example input frequency

    std::cout << "Instantiating oscillator at position (" << x << ", " << y << ")" << std::endl;
    Modules.push_back(obj);
}

void applyDrag(std::vector<Module> &Modules)
{
    for (size_t i = 0; i < Modules.size(); ++i)
    {
        auto &obj = Modules[i];

        if (isMouseHeld && (selectedObject == -1 || selectedObject == i))
        {
            selectedObject = i;
            obj.position = Vec2(xpos, ypos);
            break;
        }
    }
}

void physicsProcess(std::vector<Module> &Modules, AudioData *audioData, float dt)
{
    const int sub_steps = 2;
    const float sub_dt = dt / sub_steps;

    for (int i = 0; i < sub_steps; i++)
    {
        applyDrag(Modules);
    }
}
