#include <map>
#include "AddressMode.hpp"
#include "../Executioner.hpp"

namespace CPU
{
    namespace Instructions
    {
        AddressMode::AddressMode()
        {
            using I = CPU::Executioner;
            AddressingModes addrMode;

            lookup = {
                {addrMode.Accumulator, {&I::ACC, "ACC"}},
                {addrMode.Implied, {&I::IMP, "IMP"}},
                {addrMode.Immediate, {&I::IMM, "IMM"}},
                {addrMode.Relative, {&I::REL, "REL"}},
                {addrMode.Absolute, {&I::ABS, "ABS"}},
                {addrMode.AbsoluteX, {&I::ABX, "ABX"}},
                {addrMode.AbsoluteY, {&I::ABY, "ABY"}},
                {addrMode.ZeroPage, {&I::ZP0, "ZP0"}},
                {addrMode.ZeroPageX, {&I::ZPX, "ZPX"}},
                {addrMode.ZeroPageY, {&I::ZPY, "ZPY"}},
                {addrMode.Indirect, {&I::IND, "IND"}},
                {addrMode.IndirectX, {&I::IZX, "IZX"}},
                {addrMode.IndirectY, {&I::IZY, "IZY"}}
            };
        }

        uint8_t AddressMode::exists(uint8_t opcode)
        {
            return !(lookup.find(opcode) == lookup.end());
        }

        Executioner::tMode AddressMode::get(uint8_t opcode)
        {
            return lookup[opcode];
        }
    };
};