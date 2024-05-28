#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <memory>

// Constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int RECT_WIDTH = 100;
const int RECT_HEIGHT = 100;
const int SAMPLE_RATE = 44100;
const int AMPLITUDE = 28000;
const float INTERPOLATION_SPEED = 0.4f;
const float EPSILON = 20.0f; // Threshold for snapping to the exact position

class Node
{
public:
    SDL_Rect bottomRect;
    SDL_Rect targetRect;
    double angle;
    double scale;
    bool connected;
    Node *connectedTo;

    Node(int x, int y)
        : angle(0), scale(1.0), connected(false), connectedTo(nullptr)
    {
        bottomRect = {x, y, RECT_WIDTH, RECT_HEIGHT};
        targetRect = {x, y, RECT_WIDTH, RECT_HEIGHT};
    }

    virtual ~Node() = default;

    virtual double process(double inputFrequency) = 0;

    void updatePosition(float speed)
    {
        float distX = bottomRect.x - targetRect.x;
        float distY = bottomRect.y - targetRect.y;
        float distance = std::sqrt(distX * distX + distY * distY);

        if (std::abs(distX) < EPSILON && std::abs(distY) < EPSILON)
        {
            targetRect.x = bottomRect.x;
            targetRect.y = bottomRect.y;
        }
        else
        {
            targetRect.x += static_cast<int>(speed * distX);
            targetRect.y += static_cast<int>(speed * distY);
        }

        angle = distX / 5.0; // Update the angle based on the distance
    }

    void draw(SDL_Renderer *renderer, SDL_Texture *texture)
    {
        SDL_Rect scaledRect = {
            targetRect.x,
            targetRect.y,
            static_cast<int>(RECT_WIDTH * scale),
            static_cast<int>(RECT_HEIGHT * scale)};
        SDL_RenderCopyEx(renderer, texture, nullptr, &scaledRect, angle, nullptr, SDL_FLIP_NONE);
    }
};

class OscillatorNode : public Node
{
public:
    double frequency;

    OscillatorNode(int x, int y, double freq)
        : Node(x, y), frequency(freq) {}

    double process(double inputFrequency) override
    {
        return frequency;
    }
};

class OutputNode : public Node
{
public:
    OutputNode(int x, int y)
        : Node(x, y) {}

    double process(double inputFrequency) override
    {
        return inputFrequency;
    }
};

std::vector<std::unique_ptr<Node>> nodes;
Node *connectingNode = nullptr;

void audio_callback(void *userdata, Uint8 *stream, int len)
{
    static double phase = 0.0;
    Sint16 *buffer = (Sint16 *)stream;
    int length = len / 2;

    for (int i = 0; i < length; ++i)
    {
        double sample = 0.0;
        for (const auto &node : nodes)
        {
            if (node->connected)
            {
                double frequency = node->process(0.0); // Start processing from the node
                if (node->connectedTo)
                {
                    frequency = node->connectedTo->process(frequency);
                }
                sample += AMPLITUDE * sin(2.0 * M_PI * frequency * phase / SAMPLE_RATE);
            }
        }
        buffer[i] = static_cast<Sint16>(sample / nodes.size());
        phase += 1.0;
        if (phase >= SAMPLE_RATE)
        {
            phase -= SAMPLE_RATE;
        }
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

SDL_Texture *createTransparentTexture(SDL_Renderer *renderer, int width, int height)
{
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); // Transparent color
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
    SDL_Texture *transparentTexture = createTransparentTexture(ren, RECT_WIDTH, RECT_HEIGHT);

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
    Node *draggingNode = nullptr;
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
                    for (auto &node : nodes)
                    {
                        if (e.button.x >= node->bottomRect.x && e.button.x <= node->bottomRect.x + node->bottomRect.w &&
                            e.button.y >= node->bottomRect.y && e.button.y <= node->bottomRect.y + node->bottomRect.h)
                        {
                            dragging = true;
                            draggingNode = node.get();
                            offsetX = e.button.x - node->bottomRect.x;
                            offsetY = e.button.y - node->bottomRect.y;
                            node->scale = 1.3;
                            break;
                        }
                    }
                }
                else if (e.button.button == SDL_BUTTON_RIGHT)
                {
                    for (auto &node : nodes)
                    {
                        if (e.button.x >= node->bottomRect.x && e.button.x <= node->bottomRect.x + node->bottomRect.w &&
                            e.button.y >= node->bottomRect.y && e.button.y <= node->bottomRect.y + node->bottomRect.h)
                        {
                            if (connectingNode == nullptr)
                            {
                                connectingNode = node.get();
                            }
                            else if (connectingNode != node.get() && dynamic_cast<OscillatorNode *>(connectingNode) && dynamic_cast<OutputNode *>(node.get()))
                            {
                                connectingNode->connected = true;
                                connectingNode->connectedTo = node.get();
                                connectingNode = nullptr;
                            }
                            else if (connectingNode != node.get() && dynamic_cast<OutputNode *>(connectingNode) && dynamic_cast<OscillatorNode *>(node.get()))
                            {
                                node->connected = true;
                                node->connectedTo = connectingNode;
                                connectingNode = nullptr;
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
                    if (draggingNode != nullptr)
                    {
                        draggingNode->scale = 1.0;
                        draggingNode->bottomRect.x = ((draggingNode->bottomRect.x + RECT_WIDTH / 2) / RECT_WIDTH) * RECT_WIDTH;
                        draggingNode->bottomRect.y = ((draggingNode->bottomRect.y + RECT_HEIGHT / 2) / RECT_HEIGHT) * RECT_HEIGHT;
                        draggingNode = nullptr;
                    }
                }
            }
            else if (e.type == SDL_MOUSEMOTION)
            {
                if (dragging && draggingNode != nullptr)
                {
                    draggingNode->bottomRect.x = e.motion.x - offsetX;
                    draggingNode->bottomRect.y = e.motion.y - offsetY;
                }
            }
            else if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_o)
                {
                    int x = rand() % (WINDOW_WIDTH - RECT_WIDTH);
                    int y = rand() % (WINDOW_HEIGHT - RECT_HEIGHT);
                    double frequency = 220.0 + (y / RECT_HEIGHT) * 20.0;
                    nodes.push_back(std::unique_ptr<OscillatorNode>(new OscillatorNode(x, y, frequency)));
                }
                else if (e.key.keysym.sym == SDLK_SPACE)
                {
                    int x = rand() % (WINDOW_WIDTH - RECT_WIDTH);
                    int y = rand() % (WINDOW_HEIGHT - RECT_HEIGHT);
                    nodes.push_back(std::unique_ptr<OutputNode>(new OutputNode(x, y)));
                }
            }
        }

        // Update the position and scale of each target rectangle using linear interpolation
        for (auto &node : nodes)
        {
            node->updatePosition(INTERPOLATION_SPEED);
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

        for (const auto &node : nodes)
        {
            SDL_RenderCopy(ren, transparentTexture, nullptr, &node->bottomRect);

            if (dynamic_cast<OscillatorNode *>(node.get()))
            {
                node->draw(ren, oscillatorTexture);
            }
            else if (dynamic_cast<OutputNode *>(node.get()))
            {
                node->draw(ren, outputTexture);
            }

            if (node->connected && node->connectedTo != nullptr)
            {
                SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
                SDL_RenderDrawLine(ren,
                                   node->targetRect.x + RECT_WIDTH / 2, node->targetRect.y + RECT_HEIGHT / 2,
                                   node->connectedTo->targetRect.x + RECT_WIDTH / 2, node->connectedTo->targetRect.y + RECT_HEIGHT / 2);
            }
        }

        SDL_RenderPresent(ren);
    }

    SDL_DestroyTexture(oscillatorTexture);
    SDL_DestroyTexture(outputTexture);
    SDL_DestroyTexture(transparentTexture);
    SDL_CloseAudio();
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
