#include <array>
#include <cstdint>
#include <experimental/filesystem>
#include <fstream>
#include <memory>
#include <SDL2/SDL.h>

#include "format.h"
#include "chip8.h"

namespace fs = std::experimental::filesystem;

static bool loadROM(chip8::Chip8Context* context, const char* path)
{
    auto romPath = fs::path(path);

    if (!fs::exists(romPath)) {
        return false;
    }

    auto romSize = fs::file_size(path);
    std::vector<std::uint8_t> buffer(romSize);

    std::ifstream file(romPath, std::ios::binary);
    file.read(reinterpret_cast<char*>(buffer.data()), romSize);

    context->loadROM(buffer);

    return true;
}

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fmt::print("Couldn't init SDL: {}\n", SDL_GetError());
        return 1;
    }

    if (argc < 2) {
        fmt::print("Usage: {} <path to ROM>\n", argv[0]);
        return 1;
    }

    auto context = std::make_unique<chip8::Chip8Context>();
    if (!loadROM(context.get(), argv[1])) {
        fmt::print("Couldn't load ROM {}\n", argv[1]);
        return 1;
    }

    auto window = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>(
        SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN),
        SDL_DestroyWindow);

    auto renderer = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>(
        SDL_CreateRenderer(window.get(), -1, 0),
        SDL_DestroyRenderer);

    auto texture = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>(
        SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 16, 16),
        SDL_DestroyTexture);

    std::array<std::uint32_t, 16*16> pixels;
    auto x = 0;

    for (auto& pixel : pixels) {
        pixel = 0xFF000000 | ((x / 16) << 8) | (x % 16);
        x += 16;
    }

    SDL_UpdateTexture(texture.get(), NULL, pixels.data(), 16 * sizeof(uint32_t));

    if (!window) {
      fmt::print("Couldn't init SDL window: {}\n", SDL_GetError());
      return 1;
    }

    for (int i = 0; i < 100; i++) {
        SDL_RenderClear(renderer.get());
        SDL_RenderCopy(renderer.get(), texture.get(), NULL, NULL);
        SDL_RenderPresent(renderer.get());
    }

    SDL_Delay(5000);

    SDL_Quit();

    return 0;
}
