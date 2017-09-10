#ifndef CHIP8_H
#define CHIP8_H

#include <array>
#include <chrono>
#include <cstdint>
#include <optional>
#include <stack>
#include <vector>

namespace chip8
{
    const std::size_t MEMORY_SIZE = 0x1000;
    const std::size_t STACK_SIZE = 16;
    const std::size_t NUM_GPRS = 16;
    const std::uint16_t INITIAL_PC = 0x200;

    const std::size_t ROM_LOAD_ADDR = 0x200;
    const std::size_t ROM_MAX_SIZE = MEMORY_SIZE - ROM_LOAD_ADDR;

    const std::size_t FRAMEBUFFER_WIDTH = 64;
    const std::size_t FRAMEBUFFER_HEIGHT = 32;
    const std::size_t FRAMEBUFFER_SIZE = FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT;

    const std::chrono::milliseconds DELAY_TICK(16);

    class Chip8Context
    {
    public:
        const std::array<std::uint8_t, FRAMEBUFFER_SIZE>& getFramebuffer() const
        {
            return m_framebuffer;
        }

        void loadROM(const std::vector<std::uint8_t>& buffer);
        void tick();

    private:
        struct
        {
            std::array<std::uint8_t, NUM_GPRS> V = {{ 0 }};
            std::uint16_t I = 0;
            std::uint16_t PC = INITIAL_PC;
            std::uint16_t DT = 0;
        } m_registers;

        std::stack<std::uint16_t> m_stack;
        std::array<std::uint8_t, MEMORY_SIZE> m_memory = {{ 0 }};
        std::array<std::uint8_t, FRAMEBUFFER_SIZE> m_framebuffer = {{ 0 }};

        std::chrono::high_resolution_clock::time_point m_lastTick = std::chrono::high_resolution_clock::now();

        using InstructionHandler = std::optional<std::uint16_t>(Chip8Context::*)(std::uint16_t);
        static const std::array<InstructionHandler, 16> instructionHandlers;

        void warnUnknownInstruction(std::uint16_t instruction);

        std::optional<std::uint16_t> handle0(std::uint16_t instruction);
        std::optional<std::uint16_t> handleJP(std::uint16_t instruction);
        std::optional<std::uint16_t> handleCALL(std::uint16_t instruction);
        std::optional<std::uint16_t> handleSE(std::uint16_t instruction);
        std::optional<std::uint16_t> handleSNE(std::uint16_t instruction);
        std::optional<std::uint16_t> handleDRW(std::uint16_t instruction);
        std::optional<std::uint16_t> handleLDI(std::uint16_t instruction);
        std::optional<std::uint16_t> handleLD(std::uint16_t instruction);
        std::optional<std::uint16_t> handleADD(std::uint16_t instruction);
        std::optional<std::uint16_t> handleRND(std::uint16_t instruction);
        std::optional<std::uint16_t> handleF(std::uint16_t instruction);
        std::optional<std::uint16_t> handle8(std::uint16_t instruction);

    };
}

#endif
