#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cmath>

// Constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int RECT_WIDTH = 100;
const int RECT_HEIGHT = 100;
const int SAMPLE_RATE = 44100;
const int AMPLITUDE = 28000;
const float INTERPOLATION_SPEED = 0.3f;

float distance;

// Structure to hold pairs of bottom and target rectangles
struct RectPair
{
    SDL_Rect bottomRect;
    SDL_Rect targetRect;
    double frequency;
};

std::vector<RectPair> rectanglePairs;

void audio_callback(void *userdata, Uint8 *stream, int len)
{
    static double phase = 0.0;
    Sint16 *buffer = (Sint16 *)stream;
    int length = len / 2;

    for (int i = 0; i < length; ++i)
    {
        double sample = 0.0;
        for (const RectPair &rectPair : rectanglePairs)
        {
            sample += AMPLITUDE * sin(2.0 * M_PI * rectPair.frequency * phase / SAMPLE_RATE);
        }
        buffer[i] = static_cast<Sint16>(sample / rectanglePairs.size());
        phase += 1.0;
        if (phase >= SAMPLE_RATE)
        {
            phase -= SAMPLE_RATE;
        }
    }
}

void updateTargetRectPosition(const SDL_Rect &sourceRect, SDL_Rect &targetRect, float speed)
{
    targetRect.x += static_cast<int>(speed * (sourceRect.x - targetRect.x));
    targetRect.y += static_cast<int>(speed * (sourceRect.y - targetRect.y));
}

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("SDL2 2D Render with Audio", 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (win == nullptr)
    {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr)
    {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    SDL_AudioSpec want, have;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 4096;
    want.callback = audio_callback;

    if (SDL_OpenAudio(&want, &have) != 0)
    {
        std::cerr << "SDL_OpenAudio Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    SDL_PauseAudio(0);

    bool quit = false;
    SDL_Event e;
    bool dragging = false;
    RectPair *draggingRectPair = nullptr;
    int offsetX = 0, offsetY = 0;

    while (!quit)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                if (e.button.button == SDL_BUTTON_LEFT)
                {
                    for (RectPair &rectPair : rectanglePairs)
                    {
                        if (e.button.x >= rectPair.bottomRect.x && e.button.x <= rectPair.bottomRect.x + rectPair.bottomRect.w &&
                            e.button.y >= rectPair.bottomRect.y && e.button.y <= rectPair.bottomRect.y + rectPair.bottomRect.h)
                        {
                            dragging = true;
                            draggingRectPair = &rectPair;
                            offsetX = e.button.x - rectPair.bottomRect.x;
                            offsetY = e.button.y - rectPair.bottomRect.y;
                            break;
                        }
                    }
                }
            }
            else if (e.type == SDL_MOUSEBUTTONUP)
            {
                if (e.button.button == SDL_BUTTON_LEFT)
                {
                    dragging = false;
                    if (draggingRectPair != nullptr)
                    {
                        draggingRectPair->bottomRect.x = ((draggingRectPair->bottomRect.x + RECT_WIDTH / 2) / RECT_WIDTH) * RECT_WIDTH;
                        draggingRectPair->bottomRect.y = ((draggingRectPair->bottomRect.y + RECT_HEIGHT / 2) / RECT_HEIGHT) * RECT_HEIGHT;
                        draggingRectPair->frequency = 220.0 + (draggingRectPair->bottomRect.y / RECT_HEIGHT) * 20.0;
                        draggingRectPair = nullptr;
                    }
                    for (RectPair &rectPair : rectanglePairs)
                    {
                        distance = sqrt(pow(rectPair.bottomRect.x - rectPair.targetRect.x, 2) + pow(rectPair.bottomRect.y - rectPair.targetRect.y, 2));
                        if (distance < 100)
                        {
                            rectPair.targetRect.x = rectPair.bottomRect.x;
                            rectPair.targetRect.y = rectPair.bottomRect.y;
                        }
                    }
                }
            }
            else if (e.type == SDL_MOUSEMOTION)
            {
                if (dragging && draggingRectPair != nullptr)
                {
                    draggingRectPair->bottomRect.x = e.motion.x - offsetX;
                    draggingRectPair->bottomRect.y = e.motion.y - offsetY;
                }
            }
            else if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_SPACE)
                {
                    SDL_Rect newRect = {e.button.x, e.button.y, RECT_WIDTH, RECT_HEIGHT};
                    SDL_Rect newTargetRect = {newRect.x, newRect.y, RECT_WIDTH, RECT_HEIGHT};
                    double frequency = 220.0 + (newRect.y / RECT_HEIGHT) * 20.0;
                    rectanglePairs.push_back({newRect, newTargetRect, frequency});
                }
            }
        }

        // Update the position of each target rectangle using linear interpolation
        for (RectPair &rectPair : rectanglePairs)
        {
            updateTargetRectPosition(rectPair.bottomRect, rectPair.targetRect, INTERPOLATION_SPEED);
        }

        SDL_SetRenderDrawColor(ren, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(ren);

        SDL_SetRenderDrawColor(ren, 0x33, 0x33, 0x33, 0xFF);
        for (int x = 0; x < WINDOW_WIDTH; x += RECT_WIDTH)
        {
            for (int y = 0; y < WINDOW_HEIGHT; y += RECT_HEIGHT)
            {
                SDL_Rect gridRect = {x, y, RECT_WIDTH, RECT_HEIGHT};
                SDL_RenderDrawRect(ren, &gridRect);
            }
        }

        // Draw the bottom rectangles
        SDL_SetRenderDrawColor(ren, 0xFF, 0x00, 0x00, 0xFF);
        for (const RectPair &rectPair : rectanglePairs)
        {
            SDL_RenderFillRect(ren, &rectPair.bottomRect);
        }

        // Draw the target rectangles on top
        SDL_SetRenderDrawColor(ren, 0x00, 0xFF, 0x00, 0xFF);
        for (const RectPair &rectPair : rectanglePairs)
        {
            SDL_RenderFillRect(ren, &rectPair.targetRect);
        }

        SDL_RenderPresent(ren);
    }

    SDL_CloseAudio();
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
