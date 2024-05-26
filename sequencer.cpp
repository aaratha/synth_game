#include "sequencer.h"
#include "audio.h"
#include "constants.h"
#include <cmath>

float elapsedTime = 0.0f;

Module::Module(int freq, int amp, AudioData* audioData)
    : amplitude(amp), phase(0.0), in(440.0f), out(440.0f), modifier(1.0f)
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

void Module::updateFrequency(AudioData* audioData, float elapsedTime)
{
    out = in * (1 + 0.5 * sin(elapsedTime));
    setFrequency(audioData, out);
}
