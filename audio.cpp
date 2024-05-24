#include "audio.h"
#include <cmath>
#include <iostream>
#include "constants.h"

void audio_callback(void* userData, Uint8* stream, int len)
{
    AudioData* audioData = (AudioData*)userData;
    static double phase = 0.0;

    int16_t* buffer = reinterpret_cast<int16_t*>(stream);
    int samples = len / sizeof(int16_t);

    SDL_LockMutex(audioData->mutex);
    double frequency = audioData->frequency;
    SDL_UnlockMutex(audioData->mutex);

    for (int i = 0; i < samples; i++)
    {
        buffer[i] = static_cast<int16_t>(audioData->amplitude * std::sin(phase));
        phase += 2.0 * M_PI * frequency / SAMPLE_RATE;
        if (phase >= 2.0 * M_PI)
            phase -= 2.0 * M_PI;
    }
}

void startAudioStream(AudioData* audioData)
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_AudioSpec desiredSpec;
    SDL_AudioSpec obtainedSpec;

    desiredSpec.freq = SAMPLE_RATE;
    desiredSpec.format = AUDIO_S16SYS;
    desiredSpec.channels = 1;
    desiredSpec.samples = 4096;
    desiredSpec.callback = audio_callback;
    desiredSpec.userdata = audioData;

    if (SDL_OpenAudio(&desiredSpec, &obtainedSpec) < 0)
    {
        std::cerr << "Failed to open audio: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    std::cout << "Audio device opened: frequency=" << obtainedSpec.freq 
              << ", format=" << obtainedSpec.format 
              << ", channels=" << obtainedSpec.channels 
              << ", samples=" << obtainedSpec.samples << std::endl;

    SDL_PauseAudio(0); // Start audio playback
}

void setFrequency(AudioData* audioData, double frequency)
{
    SDL_LockMutex(audioData->mutex);
    audioData->frequency = frequency;
    SDL_UnlockMutex(audioData->mutex);
}

