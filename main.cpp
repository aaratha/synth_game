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
const float INTERPOLATION_SPEED = 0.1f;
const float EPSILON = 1.0f; // Threshold for snapping to the exact position

// Enum to differentiate between oscillator and output node
enum NodeType
{
    OSCILLATOR,
    OUTPUT
};

// Structure to hold pairs of bottom and target rectangles
struct RectPair
{
    SDL_Rect bottomRect;
    SDL_Rect targetRect;
    double frequency;
    double angle; // Angle for rotation
    NodeType type;
    bool connected;
    RectPair *connectedTo; // Pointer to the connected RectPair
};

std::vector<RectPair> rectanglePairs;
RectPair *connectingRectPair = nullptr;

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
            if (rectPair.connected && rectPair.type == OSCILLATOR)
            {
                if (rectPair.connectedTo != nullptr && rectPair.connectedTo->type == OUTPUT)
                {
                    sample += AMPLITUDE * sin(2.0 * M_PI * rectPair.frequency * phase / SAMPLE_RATE);
                }
            }
        }
        buffer[i] = static_cast<Sint16>(sample / rectanglePairs.size());
        phase += 1.0;
        if (phase >= SAMPLE_RATE)
        {
            phase -= SAMPLE_RATE;
        }
    }
}

void updateTargetRectPosition(RectPair &rectPair, double speed)
{
    float distX = rectPair.bottomRect.x - rectPair.targetRect.x;
    float distY = rectPair.bottomRect.y - rectPair.targetRect.y;

    if (std::abs(distX) < EPSILON && std::abs(distY) < EPSILON)
    {
        rectPair.targetRect.x = rectPair.bottomRect.x;
        rectPair.targetRect.y = rectPair.bottomRect.y;
    }
    else
    {
        rectPair.targetRect.x += static_cast<int>(speed * distX);
        rectPair.targetRect.y += static_cast<int>(speed * distY);
    }
}

SDL_Texture *createRectangleTexture(SDL_Renderer *renderer, int width, int height, SDL_Color color)
{
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, nullptr);
    return texture;
}

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("SDL2 Synth", 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
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

    SDL_Color red = {255, 0, 0, 255};
    SDL_Color green = {0, 255, 0, 255};
    SDL_Texture *oscillatorTexture = createRectangleTexture(ren, RECT_WIDTH, RECT_HEIGHT, red);
    SDL_Texture *outputTexture = createRectangleTexture(ren, RECT_WIDTH, RECT_HEIGHT, green);

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
                else if (e.button.button == SDL_BUTTON_RIGHT)
                {
                    for (RectPair &rectPair : rectanglePairs)
                    {
                        if (e.button.x >= rectPair.bottomRect.x && e.button.x <= rectPair.bottomRect.x + rectPair.bottomRect.w &&
                            e.button.y >= rectPair.bottomRect.y && e.button.y <= rectPair.bottomRect.y + rectPair.bottomRect.h)
                        {
                            if (connectingRectPair == nullptr)
                            {
                                connectingRectPair = &rectPair;
                            }
                            else if (connectingRectPair != &rectPair && connectingRectPair->type == OSCILLATOR && rectPair.type == OUTPUT)
                            {
                                connectingRectPair->connected = true;
                                connectingRectPair->connectedTo = &rectPair;
                                connectingRectPair = nullptr;
                            }
                            else if (connectingRectPair != &rectPair && connectingRectPair->type == OUTPUT && rectPair.type == OSCILLATOR)
                            {
                                rectPair.connected = true;
                                rectPair.connectedTo = connectingRectPair;
                                connectingRectPair = nullptr;
                            }
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
                        draggingRectPair = nullptr;
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
                if (e.key.keysym.sym == SDLK_o)
                {
                    SDL_Rect newRect = {rand() % (WINDOW_WIDTH - RECT_WIDTH), rand() % (WINDOW_HEIGHT - RECT_HEIGHT), RECT_WIDTH, RECT_HEIGHT};
                    SDL_Rect newTargetRect = {newRect.x, newRect.y, RECT_WIDTH, RECT_HEIGHT};
                    double frequency = 220.0 + (newRect.y / RECT_HEIGHT) * 20.0;
                    rectanglePairs.push_back({newRect, newTargetRect, frequency, 0.0, OSCILLATOR, false, nullptr});
                }
                else if (e.key.keysym.sym == SDLK_SPACE)
                {
                    SDL_Rect newRect = {rand() % (WINDOW_WIDTH - RECT_WIDTH), rand() % (WINDOW_HEIGHT - RECT_HEIGHT), RECT_WIDTH, RECT_HEIGHT};
                    SDL_Rect newTargetRect = {newRect.x, newRect.y, RECT_WIDTH, RECT_HEIGHT};
                    rectanglePairs.push_back({newRect, newTargetRect, 0.0, 0.0, OUTPUT, false, nullptr});
                }
            }
        }

        // Update the position of each target rectangle using linear interpolation
        for (RectPair &rectPair : rectanglePairs)
        {
            updateTargetRectPosition(rectPair, INTERPOLATION_SPEED);
        }

        SDL_SetRenderDrawColor(ren, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(ren);

        SDL_SetRenderDrawColor(ren, 0x33, 0x33, 0x33, 0xFF);
        for (int x = 0; x < WINDOW_WIDTH; x += RECT_WIDTH)
        {
            for (int y = 0; y < WINDOW_HEIGHT; y += RECT_HEIGHT)
            {
                SDL_RenderDrawLine(ren, x, 0, x, WINDOW_HEIGHT);
                SDL_RenderDrawLine(ren, 0, y, WINDOW_WIDTH, y);
            }
        }

        for (const RectPair &rectPair : rectanglePairs)
        {
            if (rectPair.type == OSCILLATOR)
            {
                SDL_RenderCopy(ren, oscillatorTexture, nullptr, &rectPair.bottomRect);
            }
            else if (rectPair.type == OUTPUT)
            {
                SDL_RenderCopy(ren, outputTexture, nullptr, &rectPair.bottomRect);
            }

            if (rectPair.connected && rectPair.connectedTo != nullptr)
            {
                SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
                SDL_RenderDrawLine(ren,
                                   rectPair.bottomRect.x + RECT_WIDTH / 2, rectPair.bottomRect.y + RECT_HEIGHT / 2,
                                   rectPair.connectedTo->bottomRect.x + RECT_WIDTH / 2, rectPair.connectedTo->bottomRect.y + RECT_HEIGHT / 2);
            }
        }

        SDL_RenderPresent(ren);
    }

    SDL_DestroyTexture(oscillatorTexture);
    SDL_DestroyTexture(outputTexture);
    SDL_CloseAudio();
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
