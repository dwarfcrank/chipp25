#include <array>
#include <cstdint>
#include <vector>

namespace chip8
{
    const std::size_t MEMORY_SIZE = 0x1000;
    const std::size_t ROM_LOAD_ADDR = 0x200;
    const std::size_t ROM_MAX_SIZE = MEMORY_SIZE - ROM_LOAD_ADDR;
    const std::size_t FRAMEBUFFER_WIDTH = 64;
    const std::size_t FRAMEBUFFER_HEIGHT = 32;
    const std::size_t FRAMEBUFFER_SIZE = FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT;

    class Chip8Context
    {
    public:
        void loadROM(const std::vector<std::uint8_t>& buffer);
        void tick();

    private:
        std::array<std::uint16_t, 16> m_registers = {{ 0 }};
        std::array<std::uint8_t, MEMORY_SIZE> m_memory = {{ 0 }};
        std::array<std::uint8_t, FRAMEBUFFER_SIZE> m_framebuffer = {{ 0 }};

        using InstructionHandler = void (Chip8Context::*)(std::uint16_t);
        static const std::array<InstructionHandler, 16> instructionHandlers;

        void handleDRW(std::uint16_t instruction);

    };
}
