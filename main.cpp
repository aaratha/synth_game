#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
#include <typeinfo>

// Constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int RECT_WIDTH = 100;
const int RECT_HEIGHT = 100;
const int SAMPLE_RATE = 44100;
const int AMPLITUDE = 28000;
const float INTERPOLATION_SPEED = 0.4f;
const float SCALE_SPEED = 4.0f;
const float EPSILON = 20.0f; // Threshold for snapping to the exact position
const float dt = 0.016f;

float elapsedTime = 0.0f;
float angleOffset = 10.0f;

class Node
{
public:
    SDL_Rect bottomRect;
    SDL_Rect targetRect;
    double baseAngle;
    double oscillationAngle;
    double scale;
    bool connected;
    bool resetting;
    bool pickedUp;
    Node *connectedTo;

    Node(int x, int y)
        : baseAngle(0), oscillationAngle(0), scale(1.0), connected(false), resetting(false), pickedUp(false), connectedTo(nullptr)
    {
        bottomRect = {x, y, RECT_WIDTH, RECT_HEIGHT};
        targetRect = {x, y, RECT_WIDTH, RECT_HEIGHT};
    }

    virtual ~Node() = default;

    virtual double process(double inputSample) = 0;

    void updatePosition(float speed)
    {
        float distX = bottomRect.x - targetRect.x;
        float distY = bottomRect.y - targetRect.y;

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

        baseAngle = distX / 5.0; // Update the base angle based on the distance
    }

    void pickUpAnimation(float elapsedTime)
    {
        if (scale < 1.3f)
        {
            scale += SCALE_SPEED * dt;
        }
        if (angleOffset > 0.0f)
        {
            angleOffset -= 20.0f * dt;
            oscillationAngle = angleOffset * sin(30 * elapsedTime);
        }
    }

    void resetAnimation()
    {
        if (scale > 1.0)
        {
            scale -= SCALE_SPEED * dt;
            if (scale < 1.0)
            {
                scale = 1.0;
            }
        }
        oscillationAngle = 0.0;
        angleOffset = 10.0f;
    }

    double getAngle() const
    {
        return baseAngle + oscillationAngle;
    }

    void draw(SDL_Renderer *renderer, SDL_Texture *texture)
    {
        SDL_Rect scaledRect = {
            targetRect.x,
            targetRect.y,
            static_cast<int>(RECT_WIDTH * scale),
            static_cast<int>(RECT_HEIGHT * scale)};
        SDL_RenderCopyEx(renderer, texture, nullptr, &scaledRect, getAngle(), nullptr, SDL_FLIP_NONE);
    }
};

class OscillatorNode : public Node
{
public:
    double frequency;
    double phase;

    OscillatorNode(int x, int y, double freq)
        : Node(x, y), frequency(freq), phase(0.0) {}

    double process(double sample) override
    {
        sample = AMPLITUDE * sin(2.0 * M_PI * frequency * phase / SAMPLE_RATE);
        phase += frequency / SAMPLE_RATE;
        if (phase >= 1.0)
            phase -= 1.0;
        return sample;
    }
};

class LFO : public Node
{
public:
    double frequency;
    double phase;

    LFO(int x, int y, double freq)
        : Node(x, y), frequency(freq), phase(0.0) {}

    double process(double sample) override
    {
        phase += frequency / SAMPLE_RATE;
        if (phase >= 1.0)
            phase -= 1.0;
        double modulation = 20 * sin(2.0 * M_PI * phase);
        // Modulate the sample with a subtle depth
        return sample * (1.0 + modulation); // Adjust the modulation depth as needed
    }
};

class OutputNode : public Node
{
public:
    OutputNode(int x, int y)
        : Node(x, y) {}

    double process(double sample) override
    {
        return sample;
    }
};

std::vector<std::unique_ptr<Node>> nodes;
Node *connectingNode = nullptr;

void audio_callback(void *userdata, Uint8 *stream, int len)
{
    Sint16 *buffer = (Sint16 *)stream;
    int length = len / 2;

    // Initialize the buffer with zeros
    for (int i = 0; i < length; ++i)
    {
        buffer[i] = 0;
    }

    // Find the output node
    OutputNode *outputNode = nullptr;
    for (const auto &node : nodes)
    {
        if (dynamic_cast<OutputNode *>(node.get()))
        {
            outputNode = static_cast<OutputNode *>(node.get());
            break;
        }
    }

    // Process the audio only if the output node is found and connected
    if (outputNode && outputNode->connected)
    {
        for (int i = 0; i < length; ++i)
        {
            double sample = 0.0;
            Node *currentNode = outputNode->connectedTo;
            while (currentNode)
            {
                sample = currentNode->process(sample);
                currentNode = currentNode->connectedTo;
            }
            buffer[i] += static_cast<Sint16>(sample);
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
    SDL_Color blue = {0, 0, 255, 255};
    SDL_Texture *oscillatorTexture = createRectangleTexture(ren, RECT_WIDTH, RECT_HEIGHT, red);
    SDL_Texture *outputTexture = createRectangleTexture(ren, RECT_WIDTH, RECT_HEIGHT, green);
    SDL_Texture *lfoTexture = createRectangleTexture(ren, RECT_WIDTH, RECT_HEIGHT, blue);
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

    // Add an initial OutputNode
    nodes.push_back(std::unique_ptr<OutputNode>(new OutputNode(2 * RECT_WIDTH, 2 * RECT_HEIGHT)));
    OutputNode *outputNode = static_cast<OutputNode *>(nodes.back().get());

    while (!quit)
    {
        elapsedTime += dt;
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
                            draggingNode->pickedUp = true;
                            offsetX = e.button.x - node->bottomRect.x;
                            offsetY = e.button.y - node->bottomRect.y;
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
                            else if (connectingNode != node.get())
                            {
                                // Disconnect previous connections if any
                                connectingNode->connected = true;
                                connectingNode->connectedTo = node.get();
                                std::cout << "Connected node: " << typeid(*connectingNode).name() << " to " << typeid(*(connectingNode->connectedTo)).name() << std::endl;
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
                        draggingNode->resetting = true;
                        draggingNode->pickedUp = false;
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
                    std::cout << "Added OscillatorNode at (" << x << ", " << y << ") with frequency " << frequency << std::endl;
                }
                else if (e.key.keysym.sym == SDLK_l)
                {
                    int x = rand() % (WINDOW_WIDTH - RECT_WIDTH);
                    int y = rand() % (WINDOW_HEIGHT - RECT_HEIGHT);
                    double frequency = 1.0 + (y / RECT_HEIGHT) * 0.1;
                    nodes.push_back(std::unique_ptr<LFO>(new LFO(x, y, frequency)));
                    std::cout << "Added LFO at (" << x << ", " << y << ") with frequency " << frequency << std::endl;
                }
                else if (e.key.keysym.sym == SDLK_SPACE)
                {
                    int x = rand() % (WINDOW_WIDTH - RECT_WIDTH);
                    int y = rand() % (WINDOW_HEIGHT - RECT_HEIGHT);
                    nodes.push_back(std::unique_ptr<OutputNode>(new OutputNode(x, y)));
                    std::cout << "Added OutputNode at (" << x << ", " << y << ")" << std::endl;
                }
            }
        }

        // Update the position and scale of each target rectangle using linear interpolation
        for (auto &node : nodes)
        {
            node->updatePosition(INTERPOLATION_SPEED);
            if (node->resetting)
            {
                node->resetAnimation();
                if (node->scale == 1.0)
                {
                    node->resetting = false;
                }
            }
            if (node->pickedUp)
            {
                node->pickUpAnimation(elapsedTime);
            }
        }

        SDL_SetRenderDrawColor(ren, 0x00, 0x33, 0x33, 0xFF);
        SDL_RenderClear(ren);

        SDL_SetRenderDrawColor(ren, 0x00, 0x33, 0x33, 0xFF);
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
            else if (dynamic_cast<LFO *>(node.get()))
            {
                node->draw(ren, lfoTexture);
            }

            // Draw connections for all nodes
            Node *currentNode = node.get();
            while (currentNode && currentNode->connectedTo)
            {
                SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
                SDL_RenderDrawLine(ren,
                                   currentNode->targetRect.x + RECT_WIDTH / 2, currentNode->targetRect.y + RECT_HEIGHT / 2,
                                   currentNode->connectedTo->targetRect.x + RECT_WIDTH / 2, currentNode->connectedTo->targetRect.y + RECT_HEIGHT / 2);
                currentNode = currentNode->connectedTo;
            }
        }

        SDL_RenderPresent(ren);
    }

    SDL_DestroyTexture(oscillatorTexture);
    SDL_DestroyTexture(outputTexture);
    SDL_DestroyTexture(lfoTexture);
    SDL_DestroyTexture(transparentTexture);
    SDL_CloseAudio();
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
