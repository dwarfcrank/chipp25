#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <chrono>
#include <optional>

#include "chip8.h"
#include "format.h"

namespace chip8
{
    const std::array<Chip8Context::InstructionHandler, 16> Chip8Context::instructionHandlers = {{
        &Chip8Context::handle0,
        &Chip8Context::handleJP,
        &Chip8Context::handleCALL,
        &Chip8Context::handleSE,
        &Chip8Context::handleSNE,
        nullptr,
        &Chip8Context::handleLD,
        &Chip8Context::handleADD,
        &Chip8Context::handle8,
        nullptr,
        &Chip8Context::handleLDI,
        nullptr,
        &Chip8Context::handleRND,
        &Chip8Context::handleDRW,
        nullptr,
        &Chip8Context::handleF,
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
        auto now = std::chrono::high_resolution_clock::now();
        if (m_registers.DT > 0 && now - m_lastTick >= DELAY_TICK) {
            m_registers.DT -= 1;
            m_lastTick = now;
        }

        auto mem = m_memory.begin() + m_registers.PC;
        std::uint16_t instruction = (*mem << 8) | *(mem + 1);

        auto op = (instruction & 0xF000) >> 12;
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
            assert(!m_stack.empty());

            auto nextPc = m_stack.top();
            m_stack.pop();

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

    std::optional<std::uint16_t> Chip8Context::handleCALL(std::uint16_t instruction)
    {
        assert(m_stack.size() < STACK_SIZE);

        auto retAddr = m_registers.PC + sizeof(instruction);
        m_stack.push(retAddr);

        return instruction & 0x0FFF;
    }

    std::optional<std::uint16_t> Chip8Context::handleSE(std::uint16_t instruction)
    {
        auto reg = (instruction & 0x0F00) >> 8;
        auto value = instruction & 0x00FF;

        if (m_registers.V[reg] == value) {
            return m_registers.PC + sizeof(instruction) * 2;
        }

        return {};
    }

    std::optional<std::uint16_t> Chip8Context::handleSNE(std::uint16_t instruction)
    {
        auto reg = (instruction & 0x0F00) >> 8;
        auto value = instruction & 0x00FF;

        if (m_registers.V[reg] != value) {
            return m_registers.PC + sizeof(instruction) * 2;
        }

        return {};
    }

    std::optional<std::uint16_t> Chip8Context::handleRND(std::uint16_t instruction)
    {
        auto reg = (instruction & 0x0F00) >> 8;
        auto value = instruction & 0x00FF;

        m_registers.V[reg] = std::rand() & value;

        return {};
    }

    std::optional<std::uint16_t> Chip8Context::handleDRW(std::uint16_t instruction)
    {
        const auto x = m_registers.V.at((instruction & 0x0F00) >> 8);
        const auto y = m_registers.V.at((instruction & 0x00F0) >> 4);
        const auto rows = instruction & 0x000F;

        m_registers.V[0xF] = 0;

        for (auto i = 0; i < rows; i++) {
            auto sprite = m_memory.at(m_registers.I + i);

            for (auto j = 7; j >= 0; j--) {
                const auto px = (x + j) % FRAMEBUFFER_WIDTH;
                const auto py = (y + i) % FRAMEBUFFER_HEIGHT;
                const auto offset = py * FRAMEBUFFER_WIDTH + px;
                const std::uint8_t oldPixel = m_framebuffer.at(offset);

                m_framebuffer.at(offset) ^= 0x00 - (sprite & 1);
                if (oldPixel == 0xFF && m_framebuffer.at(offset) == 0x00) {
                    m_registers.V[0xF] = 1;
                } 

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

    std::optional<std::uint16_t> Chip8Context::handleF(std::uint16_t instruction)
    {
        switch (instruction & 0xF0FF) {
        case 0xF015: {
            // LD DT, Vx
            auto reg = (instruction & 0x0F00) >> 8;
            m_registers.DT = m_registers.V[reg];

            break;
        }

        case 0xF007: {
            // LD Vx, DT
            auto reg = (instruction & 0x0F00) >> 8;
            m_registers.V[reg] = m_registers.DT;

            break;
        }

        case 0xF018:
            // LD ST, Vx
            break;

        case 0xF00A:
            // LD Vx, ST
            break;
        default:
            warnUnknownInstruction(instruction);
            break;
        }

        return {};
    }

    std::optional<std::uint16_t> Chip8Context::handle8(std::uint16_t instruction)
    {
        auto regA = (instruction & 0x0F00) >> 8;
        auto regB = (instruction & 0x00F0) >> 4;
        auto subOp = instruction & 0x000F;

        switch (subOp) {
        case 0: m_registers.V[regA]  = m_registers.V[regB]; break;
        case 1: m_registers.V[regA] |= m_registers.V[regB]; break;
        case 2: m_registers.V[regA] &= m_registers.V[regB]; break;
        case 3: m_registers.V[regA] ^= m_registers.V[regB]; break;

        case 4: {
            auto temp = m_registers.V[regA] + m_registers.V[regB];
            m_registers.V[0xF] = (temp & 0xFF00) ? 1 : 0;
            m_registers.V[regA] = temp & 0x00FF;
            break;
        }

        case 5: {
            m_registers.V[0xF] = (m_registers.V[regA] > m_registers.V[regB]) ? 1 : 0;
            m_registers.V[regA] -= m_registers.V[regB];
            break;
        }

        case 6: {
            m_registers.V[0xF] = m_registers.V[regA] & 1;
            m_registers.V[regA] >>= 1;
            break;
        }

        case 7: {
            m_registers.V[0xF] = (m_registers.V[regB] > m_registers.V[regA]) ? 1 : 0;
            m_registers.V[regA] = m_registers.V[regB] - m_registers.V[regA];
            break;
        }

        case 0xE: {
            m_registers.V[0xF] = m_registers.V[regA] & 0x8000;
            m_registers.V[regA] <<= 1;
            break;
        }

        default:
            warnUnknownInstruction(instruction);
            break;
        }
    }
}
