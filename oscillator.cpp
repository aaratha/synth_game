#include "oscillator.h"
#include "audio.h"
#include "constants.h"
#include <cmath>

void Oscillator::updateFrequency(AudioData* audioData, float elapsedTime)
{
    out = in * (1 + 0.5 * sin(elapsedTime)); // Modulate frequency with a sine wave
    setFrequency(audioData, out);
}

