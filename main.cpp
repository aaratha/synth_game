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

// Structure to hold rectangle data and its corresponding frequency
struct RectData
{
    SDL_Rect rect;
    double frequency;
};

std::vector<RectData> rectangles;

void audio_callback(void *userdata, Uint8 *stream, int len)
{
    static double phase = 0.0;
    Sint16 *buffer = (Sint16 *)stream;
    int length = len / 2;

    for (int i = 0; i < length; ++i)
    {
        double sample = 0.0;
        for (const RectData &rectData : rectangles)
        {
            sample += AMPLITUDE * sin(2.0 * M_PI * rectData.frequency * phase / SAMPLE_RATE);
        }
        buffer[i] = static_cast<Sint16>(sample / rectangles.size());
        phase += 1.0;
        if (phase >= SAMPLE_RATE)
        {
            phase -= SAMPLE_RATE;
        }
    }
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
                    for (RectData &rectData : rectangles)
                    {
                        if (e.button.x >= rectData.rect.x && e.button.x <= rectData.rect.x + rectData.rect.w &&
                            e.button.y >= rectData.rect.y && e.button.y <= rectData.rect.y + rectData.rect.h)
                        {
                            dragging = true;
                            offsetX = e.button.x - rectData.rect.x;
                            offsetY = e.button.y - rectData.rect.y;
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
                    for (RectData &rectData : rectangles)
                    {
                        rectData.rect.x = ((rectData.rect.x + RECT_WIDTH / 2) / RECT_WIDTH) * RECT_WIDTH;
                        rectData.rect.y = ((rectData.rect.y + RECT_HEIGHT / 2) / RECT_HEIGHT) * RECT_HEIGHT;
                        rectData.frequency = 220.0 + (rectData.rect.y / RECT_HEIGHT) * 20.0;
                    }
                }
            }
            else if (e.type == SDL_MOUSEMOTION)
            {
                if (dragging)
                {
                    for (RectData &rectData : rectangles)
                    {
                        if (e.motion.x >= rectData.rect.x && e.motion.x <= rectData.rect.x + rectData.rect.w &&
                            e.motion.y >= rectData.rect.y && e.motion.y <= rectData.rect.y + rectData.rect.h)
                        {
                            rectData.rect.x = e.motion.x - offsetX;
                            rectData.rect.y = e.motion.y - offsetY;
                            break;
                        }
                    }
                }
            }
            else if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_SPACE)
                {
                    SDL_Rect newRect = {e.button.x, e.button.y, RECT_WIDTH, RECT_HEIGHT};
                    double frequency = 220.0 + (newRect.y / RECT_HEIGHT) * 20.0;
                    rectangles.push_back(RectData{newRect, frequency});
                }
            }
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

        SDL_SetRenderDrawColor(ren, 0xFF, 0x00, 0x00, 0xFF);
        for (const RectData &rectData : rectangles)
        {
            SDL_RenderFillRect(ren, &rectData.rect);
        }

        SDL_RenderPresent(ren);
    }

    SDL_CloseAudio();
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
