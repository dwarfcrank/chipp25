#include <array>
#include <cstdint>
#include <memory>
#include <SDL2/SDL.h>

#include "format.h"

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fmt::print("Couldn't init SDL: {}\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture *texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC,
        16, 16);

    std::array<std::uint32_t, 16*16> pixels;
    auto x = 0;

    for (auto& pixel : pixels) {
        pixel = 0xFF000000 | ((x / 16) << 8) | (x % 16);
        x += 16;
    }

    SDL_UpdateTexture(texture, NULL, pixels.data(), 16 * sizeof(uint32_t));

    if (!window) {
        fmt::print("Couldn't init SDL window: {}\n", SDL_GetError());
        return 1;
    }

    for (int i = 0; i < 100; i++) {
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_Delay(5000);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
