#include <array>
#include <algorithm>
#include <cstdint>
#include <optional>

#include "chip8.h"
#include "format.h"

namespace chip8
{
    const std::array<Chip8Context::InstructionHandler, 16> Chip8Context::instructionHandlers = {{
        &Chip8Context::handle0,
        &Chip8Context::handleJP,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        &Chip8Context::handleLD,
        &Chip8Context::handleADD,
        nullptr,
        nullptr,
        &Chip8Context::handleLDI,
        nullptr,
        nullptr,
        &Chip8Context::handleDRW,
        nullptr,
        nullptr,
    }};

    void Chip8Context::loadROM(const std::vector<std::uint8_t>& buffer)
    {
        if (buffer.size() > ROM_MAX_SIZE) {
            fmt::print("Warning: ROM size {} exceeds max size {}\n", buffer.size(), ROM_MAX_SIZE);
        }

        auto num = std::min(buffer.size(), ROM_MAX_SIZE);
        auto dest = m_memory.begin() + ROM_LOAD_ADDR;

        std::copy_n(buffer.cbegin(), num, dest);
    }

    void Chip8Context::warnUnknownInstruction(std::uint16_t instruction)
    {
        fmt::print("W: Unknown instruction {:#04x} at {:#04x}\n", instruction, m_registers.PC);
    }

    void Chip8Context::tick()
    {
        auto mem = m_memory.begin() + m_registers.PC;
        std::uint16_t instruction = (*mem << 8) | *(mem + 1);

        auto op = (instruction >> 12) & 0xF;
        auto handler = instructionHandlers.at(op);

        std::optional<std::uint16_t> newPc;

        if (handler) {
            newPc = (this->*instructionHandlers[op])(instruction);
        } else {
            warnUnknownInstruction(instruction);
        }

        m_registers.PC = newPc.value_or(m_registers.PC + sizeof(instruction));
    }

    std::optional<std::uint16_t> Chip8Context::handle0(std::uint16_t instruction)
    {
        if (instruction == 0x00E0) {
            // CLS
            m_framebuffer = {{ 0 }};
        } else if (instruction == 0x00EE) {
            // RET
            auto nextPc = m_stack.at(m_registers.SP);
            m_registers.SP--;

            return nextPc;
        } else {
            warnUnknownInstruction(instruction);
        }

        return {};
    }

    std::optional<std::uint16_t> Chip8Context::handleJP(std::uint16_t instruction)
    {
        return instruction & 0x0FFF;
    }

    std::optional<std::uint16_t> Chip8Context::handleDRW(std::uint16_t instruction)
    {
        const auto x = m_registers.V.at((instruction & 0x0F00) >> 8);
        const auto y = m_registers.V.at((instruction & 0x00F0) >> 4);
        const auto rows = instruction & 0x000F;

        for (auto i = 0; i < rows; i++) {
            auto sprite = m_memory.at(m_registers.I + i);

            for (auto j = 7; j >= 0; j--) {
                const auto px = (x + j) % FRAMEBUFFER_WIDTH;
                const auto py = (y + i) % FRAMEBUFFER_HEIGHT;

                m_framebuffer.at(py * FRAMEBUFFER_WIDTH + px) ^= 0x00 - (sprite & 1);
                sprite >>= 1;
            }
        }

        return {};
    }

    std::optional<std::uint16_t> Chip8Context::handleLDI(std::uint16_t instruction)
    {
        m_registers.I = instruction & 0x0FFF;
        return {};
    }

    std::optional<std::uint16_t> Chip8Context::handleLD(std::uint16_t instruction)
    {
        auto reg = (instruction & 0x0F00) >> 8;
        m_registers.V[reg] = instruction & 0x00FF;

        return {};
    }

    std::optional<std::uint16_t> Chip8Context::handleADD(std::uint16_t instruction)
    {
        auto reg = (instruction & 0x0F00) >> 8;
        m_registers.V[reg] += instruction & 0x00FF;

        return {};
    }
}
