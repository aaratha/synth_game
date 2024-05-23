#ifndef AUDIO_H
#define AUDIO_H

#include <SDL2/SDL.h>
#include <vector>
#include "physics.h"

void audio_callback(void* userData, Uint8* stream, int len);

#endif // AUDIO_H

