#include <algorithm>
#include <array>
#include <cstdint>
#include <experimental/filesystem>
#include <fstream>
#include <memory>
#include <SDL2/SDL.h>

#include "format.h"
#include "chip8.h"

namespace fs = std::experimental::filesystem;

static const int WINDOW_WIDTH = 1280;
static const int WINDOW_HEIGHT = 720;

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

static void copyFramebuffer(chip8::Chip8Context* context, SDL_Texture* texture)
{
    const auto& framebuffer = context->getFramebuffer();
    std::vector<std::uint32_t> pixels(framebuffer.size());

    std::transform(framebuffer.cbegin(), framebuffer.cend(), pixels.begin(),
                   [](auto pixel) { return pixel ? 0xFFFFFFFF : 0; });

    SDL_UpdateTexture(texture, nullptr, pixels.data(), chip8::FRAMEBUFFER_WIDTH * sizeof(std::uint32_t));
}

static SDL_Rect computeDrawRect(int width, int height)
{
    SDL_Rect result;

    auto aspect = static_cast<float>(chip8::FRAMEBUFFER_WIDTH) / static_cast<float>(chip8::FRAMEBUFFER_HEIGHT);
    auto aw = static_cast<float>(height) * aspect;
    auto ah = static_cast<float>(height);

    if (aw > width) {
        aw = static_cast<float>(width);
        ah = aw / aspect;
    }

    result.x = width / 2 - aw / 2;
    result.y = height / 2 - ah / 2;
    result.w = aw;
    result.h = ah;

    return result;
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
        SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                         WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN),
        SDL_DestroyWindow);

    auto renderer = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>(
        SDL_CreateRenderer(window.get(), -1, 0),
        SDL_DestroyRenderer);

    auto texture = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>(
        SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC,
                          chip8::FRAMEBUFFER_WIDTH, chip8::FRAMEBUFFER_HEIGHT),
        SDL_DestroyTexture);

    if (!window) {
      fmt::print("Couldn't init SDL window: {}\n", SDL_GetError());
      return 1;
    }

    auto drawRect = computeDrawRect(WINDOW_WIDTH, WINDOW_HEIGHT);

    for (int i = 0; i < 100; i++) {
        context->tick();
        copyFramebuffer(context.get(), texture.get());

        SDL_RenderClear(renderer.get());
        SDL_RenderCopy(renderer.get(), texture.get(), nullptr, &drawRect);
        SDL_RenderPresent(renderer.get());
    }

    SDL_Delay(5000);

    SDL_Quit();

    return 0;
}
