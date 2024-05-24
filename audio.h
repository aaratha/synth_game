#ifndef AUDIO_H
#define AUDIO_H

#include <SDL2/SDL.h>

typedef struct {
    double frequency;
    int amplitude;
    SDL_mutex* mutex;
} AudioData;

void audio_callback(void* userData, Uint8* stream, int len);
void startAudioStream(AudioData* audioData);
void setFrequency(AudioData* audioData, double frequency);

#endif // AUDIO_H

