#include <array>
#include <algorithm>
#include <cstdint>

#include "chip8.h"
#include "format.h"

namespace chip8
{
    const std::array<Chip8Context::InstructionHandler, 16> Chip8Context::instructionHandlers = {{
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
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

    void Chip8Context::tick()
    {
        
    }

    void Chip8Context::handleDRW(std::uint16_t instruction)
    {
        
    }

}
