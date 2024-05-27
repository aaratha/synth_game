#include <SDL2/SDL.h>
#include <iostream>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int RECT_WIDTH = 100;
const int RECT_HEIGHT = 100;

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("SDL2 2D Render", 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
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

    bool quit = false;
    SDL_Event e;

    SDL_Rect rect = {WINDOW_WIDTH / 2 - RECT_WIDTH / 2, WINDOW_HEIGHT / 2 - RECT_HEIGHT / 2, RECT_WIDTH, RECT_HEIGHT};
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
                if (e.button.button == SDL_BUTTON_LEFT &&
                    e.button.x >= rect.x && e.button.x <= rect.x + rect.w &&
                    e.button.y >= rect.y && e.button.y <= rect.y + rect.h)
                {
                    dragging = true;
                    offsetX = e.button.x - rect.x;
                    offsetY = e.button.y - rect.y;
                }
            }
            else if (e.type == SDL_MOUSEBUTTONUP)
            {
                if (e.button.button == SDL_BUTTON_LEFT)
                {
                    dragging = false;
                    // Snap to grid
                    rect.x = ((rect.x + RECT_WIDTH / 2) / RECT_WIDTH) * RECT_WIDTH;
                    rect.y = ((rect.y + RECT_HEIGHT / 2) / RECT_HEIGHT) * RECT_HEIGHT;
                }
            }
            else if (e.type == SDL_MOUSEMOTION)
            {
                if (dragging)
                {
                    rect.x = e.motion.x - offsetX;
                    rect.y = e.motion.y - offsetY;
                }
            }
        }

        SDL_SetRenderDrawColor(ren, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(ren);

        // Draw grid
        SDL_SetRenderDrawColor(ren, 0x33, 0x33, 0x33, 0xFF);
        for (int x = 0; x < WINDOW_WIDTH; x += RECT_WIDTH)
        {
            for (int y = 0; y < WINDOW_HEIGHT; y += RECT_HEIGHT)
            {
                SDL_Rect gridRect = {x, y, RECT_WIDTH, RECT_HEIGHT};
                SDL_RenderDrawRect(ren, &gridRect);
            }
        }

        // Draw rectangle / 2 / 2
        SDL_SetRenderDrawColor(ren, 0xFF, 0x00, 0x00, 0xFF);
        SDL_RenderFillRect(ren, &rect);

        SDL_RenderPresent(ren);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
