#include "audio.h"
#include <cmath>
#include "physics.h"
#include "constants.h"

void audio_callback(void* userData, Uint8* stream, int len)
{
    int16_t* buffer = reinterpret_cast<int16_t*>(stream);
    int samples = len / sizeof(int16_t);

    for (int i = 0; i < samples; i++)
    {
        int sampleValue = 0;
        for (auto& module : Modules)
        {
            sampleValue += static_cast<int16_t>(module.amplitude * std::sin(module.phase));
            module.generateSound(1.0 / SAMPLE_RATE);
        }
        if (sampleValue > 32767) sampleValue = 32767;
        if (sampleValue < -32768) sampleValue = -32768;
        buffer[i] = sampleValue;
    }
}

