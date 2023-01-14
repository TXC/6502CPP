#include "Executioner.hpp"
#include "Processor.hpp"

namespace CPU
{
  void Executioner::loadInstructions()
  {
    // Assembles the translation table.
    // The table is one big initialiser list of initialiser lists...

    using I = CPU::Executioner;
    AddressingModes addrMode;
    InstructionSet instSet;

    addressModes = {
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
    operationCodes = {
      {instSet.ADC, {&I::ADC, "ADC"}}, {instSet.AND, {&I::AND, "AND"}}, {instSet.ASL, {&I::ASL, "ASL"}}, {instSet.BCC, {&I::BCC, "BCC"}},
      {instSet.BCS, {&I::BCS, "BCS"}}, {instSet.BEQ, {&I::BEQ, "BEQ"}}, {instSet.BIT, {&I::BIT, "BIT"}}, {instSet.BMI, {&I::BMI, "BMI"}},
      {instSet.BNE, {&I::BNE, "BNE"}}, {instSet.BPL, {&I::BPL, "BPL"}}, {instSet.BRK, {&I::BRK, "BRK"}}, {instSet.BVC, {&I::BVC, "BVC"}},
      {instSet.BVS, {&I::BVS, "BVS"}}, {instSet.CLC, {&I::CLC, "CLC"}}, {instSet.CLD, {&I::CLD, "CLD"}}, {instSet.CLI, {&I::CLI, "CLI"}},
      {instSet.CLV, {&I::CLV, "CLV"}}, {instSet.CMP, {&I::CMP, "CMP"}}, {instSet.CPX, {&I::CPX, "CPX"}}, {instSet.CPY, {&I::CPY, "CPY"}},
      {instSet.DEC, {&I::DEC, "DEC"}}, {instSet.DEX, {&I::DEX, "DEX"}}, {instSet.DEY, {&I::DEY, "DEY"}}, {instSet.EOR, {&I::EOR, "EOR"}},
      {instSet.INC, {&I::INC, "INC"}}, {instSet.INX, {&I::INX, "INX"}}, {instSet.INY, {&I::INY, "INY"}}, {instSet.JMP, {&I::JMP, "JMP"}},
      {instSet.JSR, {&I::JSR, "JSR"}}, {instSet.LDA, {&I::LDA, "LDA"}}, {instSet.LDX, {&I::LDX, "LDX"}}, {instSet.LDY, {&I::LDY, "LDY"}},
      {instSet.LSR, {&I::LSR, "LSR"}}, {instSet.NOP, {&I::NOP, "NOP"}}, {instSet.ORA, {&I::ORA, "ORA"}}, {instSet.PHA, {&I::PHA, "PHA"}},
      {instSet.PHP, {&I::PHP, "PHP"}}, {instSet.PLA, {&I::PLA, "PLA"}}, {instSet.PLP, {&I::PLP, "PLP"}}, {instSet.ROL, {&I::ROL, "ROL"}},
      {instSet.ROR, {&I::ROR, "ROR"}}, {instSet.RTI, {&I::RTI, "RTI"}}, {instSet.RTS, {&I::RTS, "RTS"}}, {instSet.SBC, {&I::SBC, "SBC"}},
      {instSet.SEC, {&I::SEC, "SEC"}}, {instSet.SED, {&I::SED, "SED"}}, {instSet.SEI, {&I::SEI, "SEI"}}, {instSet.STA, {&I::STA, "STA"}},
      {instSet.STX, {&I::STX, "STX"}}, {instSet.STY, {&I::STY, "STY"}}, {instSet.TAX, {&I::TAX, "TAX"}}, {instSet.TAY, {&I::TAY, "TAY"}},
      {instSet.TSX, {&I::TSX, "TSX"}}, {instSet.TXA, {&I::TXA, "TXA"}}, {instSet.TXS, {&I::TXS, "TXS"}}, {instSet.TYA, {&I::TYA, "TYA"}},
#ifdef ILLEGAL
      {instSet.ANC, {&I::ANC, "ANC"}}, {instSet.ALR, {&I::ALR, "ALR"}}, {instSet.ANC2, {&I::ANC2, "ANC2"}}, {instSet.ANE, {&I::ANE, "ANE"}},
      {instSet.ARR, {&I::ARR, "ARR"}}, {instSet.DCP, {&I::DCP, "DCP"}}, {instSet.ISC, {&I::ISC, "ISC"}}, {instSet.LAS, {&I::LAS, "LAS"}},
      {instSet.LAX, {&I::LAX, "LAX"}}, {instSet.LXA, {&I::LXA, "LXA"}}, {instSet.RLA, {&I::RLA, "RLA"}}, {instSet.RRA, {&I::RRA, "RRA"}},
      {instSet.SAX, {&I::SAX, "SAX"}}, {instSet.SBX, {&I::SBX, "SBX"}}, {instSet.SHA, {&I::SHA, "SHA"}}, {instSet.SHX, {&I::SHX, "SHX"}},
      {instSet.SHY, {&I::SHY, "SHY"}}, {instSet.SLO, {&I::SLO, "SLO"}}, {instSet.SRE, {&I::SRE, "SRE"}}, {instSet.TAS, {&I::TAS, "TAS"}},
      {instSet.USBC, {&I::USBC, "USBC"}}, {instSet.DOP, {&I::DOP, "DOP"}}, {instSet.TOP, {&I::TOP, "TOP"}}, {instSet.JAM, {&I::JAM, "JAM"}},
#else
      {instSet.XXX, {&I::XXX, "XXX"}}
#endif
    };


#ifndef ILLEGAL
      for (uint8_t z = 0x00; z <= 0xFF; z++)
      {
        lookup[z] = { instSet.XXX, addrMode.Implied, 0 };
      }
#endif

#pragma region Instructions
#pragma region Instructions 0x0
      lookup[0x00] = { instSet.BRK, addrMode.Immediate, 7 };
      lookup[0x01] = { instSet.ORA, addrMode.IndirectX, 6 };
#ifdef ILLEGAL
      lookup[0x02] = { instSet.JAM, addrMode.Immediate, 2 };
      lookup[0x03] = { instSet.SLO, addrMode.IndirectX, 8 };
      lookup[0x04] = { instSet.DOP, addrMode.ZeroPage, 3 };
#endif
      lookup[0x05] = { instSet.ORA, addrMode.ZeroPage, 3 };
      lookup[0x06] = { instSet.ASL, addrMode.ZeroPage, 5 };
#ifdef ILLEGAL
      lookup[0x07] = { instSet.SLO, addrMode.ZeroPage, 5 };
#endif
      lookup[0x08] = { instSet.PHP, addrMode.Implied, 3 };
      lookup[0x09] = { instSet.ORA, addrMode.Immediate, 2 };
      lookup[0x0A] = { instSet.ASL, addrMode.Accumulator, 2 };
#ifdef ILLEGAL
      lookup[0x0B] = { instSet.ANC, addrMode.Immediate, 2 };
      lookup[0x0C] = { instSet.TOP, addrMode.Absolute, 4 };
#endif
      lookup[0x0D] = { instSet.ORA, addrMode.Absolute, 4 };
      lookup[0x0E] = { instSet.ASL, addrMode.Absolute, 6 };
#ifdef ILLEGAL
      lookup[0x0F] = { instSet.SLO, addrMode.Absolute, 6 };
#endif
#pragma endregion Instructions 0x0

#pragma region Instructions 0x1
      lookup[0x10] = { instSet.BPL, addrMode.Relative, 2 };
      lookup[0x11] = { instSet.ORA, addrMode.IndirectY, 5 };
#ifdef ILLEGAL
      lookup[0x12] = { instSet.JAM, addrMode.Immediate, 2 };
      lookup[0x13] = { instSet.SLO, addrMode.IndirectY, 8 };
      lookup[0x14] = { instSet.DOP, addrMode.ZeroPageX, 4 };
#endif
      lookup[0x15] = { instSet.ORA, addrMode.ZeroPageX, 4 };
      lookup[0x16] = { instSet.ASL, addrMode.ZeroPageX, 6 };
#ifdef ILLEGAL
      lookup[0x17] = { instSet.SLO, addrMode.ZeroPageX, 6 };
#endif
      lookup[0x18] = { instSet.CLC, addrMode.Implied, 2 };
      lookup[0x19] = { instSet.ORA, addrMode.AbsoluteY, 4 };
#ifdef ILLEGAL
      lookup[0x1A] = { instSet.NOP, addrMode.Implied, 2 };
      lookup[0x1B] = { instSet.SLO, addrMode.AbsoluteY, 7 };
      lookup[0x1C] = { instSet.TOP, addrMode.AbsoluteX, 4 };
#endif
      lookup[0x1D] = { instSet.ORA, addrMode.AbsoluteX, 4 };
      lookup[0x1E] = { instSet.ASL, addrMode.AbsoluteX, 7 };
#ifdef ILLEGAL
      lookup[0x1F] = { instSet.SLO, addrMode.AbsoluteX, 7 };
#endif
#pragma endregion Instructions 0x1

#pragma region Instructions 0x2
      lookup[0x20] = { instSet.JSR, addrMode.Absolute, 6 };
      lookup[0x21] = { instSet.AND, addrMode.IndirectX, 6 };
#ifdef ILLEGAL
      lookup[0x22] = { instSet.JAM, addrMode.Immediate, 2 };
      lookup[0x23] = { instSet.RRA, addrMode.IndirectX, 8 };
#endif
      lookup[0x24] = { instSet.BIT, addrMode.ZeroPage, 3 };
      lookup[0x25] = { instSet.AND, addrMode.ZeroPage, 3 };
      lookup[0x26] = { instSet.ROL, addrMode.ZeroPage, 5 };
#ifdef ILLEGAL
      lookup[0x27] = { instSet.RRA, addrMode.ZeroPage, 5 };
#endif
      lookup[0x28] = { instSet.PLP, addrMode.Implied, 4 };
      lookup[0x29] = { instSet.AND, addrMode.Immediate, 2 };
      lookup[0x2A] = { instSet.ROL, addrMode.Accumulator, 2 };
#ifdef ILLEGAL
      lookup[0x2B] = { instSet.ANC2, addrMode.Immediate, 2 };
#endif
      lookup[0x2C] = { instSet.BIT, addrMode.Absolute, 4 };
      lookup[0x2D] = { instSet.AND, addrMode.Absolute, 4 };
      lookup[0x2E] = { instSet.ROL, addrMode.Absolute, 6 };
#ifdef ILLEGAL
      lookup[0x2F] = { instSet.RRA, addrMode.Absolute, 6 };
#endif
#pragma endregion Instructions 0x2

#pragma region Instructions 0x3
      lookup[0x30] = { instSet.BMI, addrMode.Relative, 2 };
      lookup[0x31] = { instSet.AND, addrMode.IndirectY, 5 };
#ifdef ILLEGAL
      lookup[0x32] = { instSet.JAM, addrMode.Immediate, 2 };
      lookup[0x33] = { instSet.RRA, addrMode.IndirectY, 8 };
      lookup[0x34] = { instSet.DOP, addrMode.ZeroPageX, 4 };
#endif
      lookup[0x35] = { instSet.AND, addrMode.ZeroPageX, 4 };
      lookup[0x36] = { instSet.ROL, addrMode.ZeroPageX, 6 };
#ifdef ILLEGAL
      lookup[0x37] = { instSet.RRA, addrMode.ZeroPageX, 6 };
#endif
      lookup[0x38] = { instSet.SEC, addrMode.Implied, 2 };
      lookup[0x39] = { instSet.AND, addrMode.AbsoluteY, 4 };
#ifdef ILLEGAL
      lookup[0x3A] = { instSet.NOP, addrMode.Implied, 2 };
      lookup[0x3B] = { instSet.RRA, addrMode.AbsoluteY, 7 };
      lookup[0x3C] = { instSet.TOP, addrMode.AbsoluteX, 4 };
#endif
      lookup[0x3D] = { instSet.AND, addrMode.AbsoluteX, 4 };
      lookup[0x3E] = { instSet.ROL, addrMode.AbsoluteX, 7 };
#ifdef ILLEGAL
      lookup[0x3F] = { instSet.RRA, addrMode.AbsoluteX, 7 };
#endif
#pragma endregion Instructions 0x3

#pragma region Instructions 0x4
      lookup[0x40] = { instSet.RTI, addrMode.Implied, 6 };
      lookup[0x41] = { instSet.EOR, addrMode.IndirectX, 6 };
#ifdef ILLEGAL
      lookup[0x42] = { instSet.JAM, addrMode.Immediate, 2 };
      lookup[0x43] = { instSet.SRE, addrMode.IndirectX, 8 };
      lookup[0x44] = { instSet.DOP, addrMode.ZeroPage, 3 };
#endif
      lookup[0x45] = { instSet.EOR, addrMode.ZeroPage, 3 };
      lookup[0x46] = { instSet.LSR, addrMode.ZeroPage, 5 };
#ifdef ILLEGAL
      lookup[0x47] = { instSet.SRE, addrMode.ZeroPage, 5 };
#endif
      lookup[0x48] = { instSet.PHA, addrMode.Implied, 3 };
      lookup[0x49] = { instSet.EOR, addrMode.Immediate, 2 };
      lookup[0x4A] = { instSet.LSR, addrMode.Accumulator, 2 };
#ifdef ILLEGAL
      lookup[0x4B] = { instSet.ALR, addrMode.Immediate, 2 };
#endif
      lookup[0x4C] = { instSet.JMP, addrMode.Absolute, 3 };
      lookup[0x4D] = { instSet.EOR, addrMode.Absolute, 4 };
      lookup[0x4E] = { instSet.LSR, addrMode.Absolute, 6 };
#ifdef ILLEGAL
      lookup[0x4F] = { instSet.SRE, addrMode.Absolute, 6 };
#endif
#pragma endregion Instructions 0x4

#pragma region Instructions 0x5
      lookup[0x50] = { instSet.BVC, addrMode.Relative, 2 };
      lookup[0x51] = { instSet.EOR, addrMode.IndirectY, 5 };
#ifdef ILLEGAL
      lookup[0x52] = { instSet.JAM, addrMode.Immediate, 2 };
      lookup[0x53] = { instSet.SRE, addrMode.IndirectY, 8 };
      lookup[0x54] = { instSet.DOP, addrMode.ZeroPageX, 4 };
#endif
      lookup[0x55] = { instSet.EOR, addrMode.ZeroPageX, 4 };
      lookup[0x56] = { instSet.LSR, addrMode.ZeroPageX, 6 };
#ifdef ILLEGAL
      lookup[0x57] = { instSet.SRE, addrMode.ZeroPageX, 6 };
#endif
      lookup[0x58] = { instSet.CLI, addrMode.Implied, 2 };
      lookup[0x59] = { instSet.EOR, addrMode.AbsoluteY, 4 };
#ifdef ILLEGAL
      lookup[0x5A] = { instSet.NOP, addrMode.Implied, 2 };
      lookup[0x5B] = { instSet.SRE, addrMode.AbsoluteY, 7 };
      lookup[0x5C] = { instSet.TOP, addrMode.AbsoluteX, 4 };
#endif
      lookup[0x5D] = { instSet.EOR, addrMode.AbsoluteX, 4 };
      lookup[0x5E] = { instSet.LSR, addrMode.AbsoluteX, 7 };
#ifdef ILLEGAL
      lookup[0x5F] = { instSet.SRE, addrMode.AbsoluteX, 7 };
#endif
#pragma endregion Instructions 0x5

#pragma region Instructions 0x6
      lookup[0x60] = { instSet.RTS, addrMode.Implied, 6 };
      lookup[0x61] = { instSet.ADC, addrMode.IndirectX, 6 };
#ifdef ILLEGAL
      lookup[0x62] = { instSet.JAM, addrMode.Immediate, 2 };
      lookup[0x63] = { instSet.RLA, addrMode.IndirectX, 8 };
      lookup[0x64] = { instSet.DOP, addrMode.ZeroPage, 3 };
#endif
      lookup[0x65] = { instSet.ADC, addrMode.ZeroPage, 3 };
      lookup[0x66] = { instSet.ROR, addrMode.ZeroPage, 5 };
#ifdef ILLEGAL
      lookup[0x67] = { instSet.RLA, addrMode.ZeroPage, 5 };
#endif
      lookup[0x68] = { instSet.PLA, addrMode.Implied, 4 };
      lookup[0x69] = { instSet.ADC, addrMode.Immediate, 2 };
      lookup[0x6A] = { instSet.ROR, addrMode.Accumulator, 2 };
#ifdef ILLEGAL
      lookup[0x6B] = { instSet.ARR, addrMode.Immediate, 2 };
#endif
      lookup[0x6C] = { instSet.JMP, addrMode.Indirect, 5 };
      lookup[0x6D] = { instSet.ADC, addrMode.Absolute, 4 };
      lookup[0x6E] = { instSet.ROR, addrMode.Absolute, 6 };
#ifdef ILLEGAL
      lookup[0x6F] = { instSet.RLA, addrMode.Absolute, 6 };
#endif
#pragma endregion Instructions 0x6

#pragma region Instructions 0x7
      lookup[0x70] = { instSet.BVS, addrMode.Relative, 2 };
      lookup[0x71] = { instSet.ADC, addrMode.IndirectY, 5 };
#ifdef ILLEGAL
      lookup[0x72] = { instSet.JAM, addrMode.Immediate, 2 };
      lookup[0x73] = { instSet.RLA, addrMode.IndirectY, 8 };
      lookup[0x74] = { instSet.DOP, addrMode.ZeroPageX, 4 };
#endif
      lookup[0x75] = { instSet.ADC, addrMode.ZeroPageX, 4 };
      lookup[0x76] = { instSet.ROR, addrMode.ZeroPageX, 6 };
#ifdef ILLEGAL
      lookup[0x77] = { instSet.RLA, addrMode.ZeroPageX, 6 };
#endif
      lookup[0x78] = { instSet.SEI, addrMode.Implied, 2 };
      lookup[0x79] = { instSet.ADC, addrMode.AbsoluteY, 4 };
#ifdef ILLEGAL
      lookup[0x7A] = { instSet.NOP, addrMode.Implied, 2 };
      lookup[0x7B] = { instSet.RLA, addrMode.AbsoluteY, 7 };
      lookup[0x7C] = { instSet.TOP, addrMode.AbsoluteX, 4 };
#endif
      lookup[0x7D] = { instSet.ADC, addrMode.AbsoluteX, 4 };
      lookup[0x7E] = { instSet.ROR, addrMode.AbsoluteX, 7 };
#ifdef ILLEGAL
      lookup[0x7F] = { instSet.RLA, addrMode.AbsoluteX, 7 };
#endif
#pragma endregion Instructions 0x7

#pragma region Instructions 0x8
#ifdef ILLEGAL
      lookup[0x80] = { instSet.DOP, addrMode.Immediate, 2 };
#endif
      lookup[0x81] = { instSet.STA, addrMode.IndirectX, 6 };
#ifdef ILLEGAL
      lookup[0x82] = { instSet.DOP, addrMode.Immediate, 2 };
      lookup[0x83] = { instSet.SAX, addrMode.IndirectX, 6 };
#endif
      lookup[0x84] = { instSet.STY, addrMode.ZeroPage, 3 };
      lookup[0x85] = { instSet.STA, addrMode.ZeroPage, 3 };
      lookup[0x86] = { instSet.STX, addrMode.ZeroPage, 3 };
#ifdef ILLEGAL
      lookup[0x87] = { instSet.SAX, addrMode.ZeroPage, 3 };
#endif
      lookup[0x88] = { instSet.DEY, addrMode.Implied, 2 };
#ifdef ILLEGAL

      lookup[0x89] = { instSet.DOP, addrMode.Immediate, 2 };
#endif
      lookup[0x8A] = { instSet.TXA, addrMode.Implied, 2 };
#ifdef ILLEGAL
      lookup[0x8B] = { instSet.ANE, addrMode.Immediate, 2 };
#endif
      lookup[0x8C] = { instSet.STY, addrMode.Absolute, 4 };
      lookup[0x8D] = { instSet.STA, addrMode.Absolute, 4 };
      lookup[0x8E] = { instSet.STX, addrMode.Absolute, 4 };
#ifdef ILLEGAL
      lookup[0x8F] = { instSet.SAX, addrMode.Absolute, 4 };
#endif
#pragma endregion Instructions 0x8

#pragma region Instructions 0x9
      lookup[0x90] = { instSet.BCC, addrMode.Relative, 2 };
      lookup[0x91] = { instSet.STA, addrMode.IndirectY, 6 };
#ifdef ILLEGAL
      lookup[0x92] = { instSet.JAM, addrMode.Immediate, 2 };
      lookup[0x93] = { instSet.SHA, addrMode.IndirectY, 6 };
#endif
      lookup[0x94] = { instSet.STY, addrMode.ZeroPageX, 4 };
      lookup[0x95] = { instSet.STA, addrMode.ZeroPageX, 4 };
      lookup[0x96] = { instSet.STX, addrMode.ZeroPageY, 4 };
#ifdef ILLEGAL
      lookup[0x97] = { instSet.SAX, addrMode.ZeroPageY, 4 };
#endif
      lookup[0x98] = { instSet.TYA, addrMode.Implied, 2 };
      lookup[0x99] = { instSet.STA, addrMode.AbsoluteY, 5 };
      lookup[0x9A] = { instSet.TXS, addrMode.Implied, 2 };
#ifdef ILLEGAL
      lookup[0x9B] = { instSet.TAS, addrMode.AbsoluteY, 5 };
      lookup[0x9C] = { instSet.SHY, addrMode.AbsoluteX, 5 };
#endif
      lookup[0x9D] = { instSet.STA, addrMode.AbsoluteX, 5 };
#ifdef ILLEGAL
      lookup[0x9E] = { instSet.SHX, addrMode.AbsoluteY, 5 };
      lookup[0x9F] = { instSet.SHA, addrMode.AbsoluteY, 5 };
#endif
#pragma endregion Instructions 0x9

#pragma region Instructions 0xA
      lookup[0xA0] = { instSet.LDY, addrMode.Immediate, 2 };
      lookup[0xA1] = { instSet.LDA, addrMode.IndirectX, 6 };
      lookup[0xA2] = { instSet.LDX, addrMode.Immediate, 2 };
#ifdef ILLEGAL
      lookup[0xA3] = { instSet.LAX, addrMode.IndirectX, 6 };
#endif
      lookup[0xA4] = { instSet.LDY, addrMode.ZeroPage, 3 };
      lookup[0xA5] = { instSet.LDA, addrMode.ZeroPage, 3 };
      lookup[0xA6] = { instSet.LDX, addrMode.ZeroPage, 3 };
#ifdef ILLEGAL
      lookup[0xA7] = { instSet.LAX, addrMode.ZeroPage, 3 };
#endif
      lookup[0xA8] = { instSet.TAY, addrMode.Implied, 2 };
      lookup[0xA9] = { instSet.LDA, addrMode.Immediate, 2 };
      lookup[0xAA] = { instSet.TAX, addrMode.Implied, 2 };
#ifdef ILLEGAL
      lookup[0xAB] = { instSet.LXA, addrMode.Immediate, 2 };
#endif
      lookup[0xAC] = { instSet.LDY, addrMode.Absolute, 4 };
      lookup[0xAD] = { instSet.LDA, addrMode.Absolute, 4 };
      lookup[0xAE] = { instSet.LDX, addrMode.Absolute, 4 };
#ifdef ILLEGAL
      lookup[0xAF] = { instSet.LAX, addrMode.Absolute, 4 };
#endif
#pragma endregion Instructions 0xA

#pragma region Instructions 0xB
      lookup[0xB0] = { instSet.BCS, addrMode.Relative, 2 };
      lookup[0xB1] = { instSet.LDA, addrMode.IndirectY, 5 };
#ifdef ILLEGAL
      lookup[0xB2] = { instSet.JAM, addrMode.Immediate, 2 };
      lookup[0xB3] = { instSet.LAX, addrMode.IndirectY, 5 };
#endif
      lookup[0xB4] = { instSet.LDY, addrMode.ZeroPageX, 4 };
      lookup[0xB5] = { instSet.LDA, addrMode.ZeroPageX, 4 };
      lookup[0xB6] = { instSet.LDX, addrMode.ZeroPageY, 4 };
#ifdef ILLEGAL
      lookup[0xB7] = { instSet.LAX, addrMode.ZeroPageX, 4 };
#endif
      lookup[0xB8] = { instSet.CLV, addrMode.Implied, 2 };
      lookup[0xB9] = { instSet.LDA, addrMode.AbsoluteY, 4 };
      lookup[0xBA] = { instSet.TSX, addrMode.Implied, 2 };
#ifdef ILLEGAL
      lookup[0xBB] = { instSet.LAS, addrMode.AbsoluteY, 4 };
#endif
      lookup[0xBC] = { instSet.LDY, addrMode.AbsoluteX, 4 };
      lookup[0xBD] = { instSet.LDA, addrMode.AbsoluteX, 4 };
      lookup[0xBE] = { instSet.LDX, addrMode.AbsoluteY, 4 };
#ifdef ILLEGAL
      lookup[0xBF] = { instSet.LAX, addrMode.AbsoluteY, 4 };
#endif
#pragma endregion Instructions 0xB

#pragma region Instructions 0xC
      lookup[0xC0] = { instSet.CPY, addrMode.Immediate, 2 };
      lookup[0xC1] = { instSet.CMP, addrMode.IndirectX, 6 };
#ifdef ILLEGAL
      lookup[0xC2] = { instSet.DOP, addrMode.Immediate, 2 };
      lookup[0xC3] = { instSet.DCP, addrMode.IndirectX, 8 };
#endif
      lookup[0xC4] = { instSet.CPY, addrMode.ZeroPage, 3 };
      lookup[0xC5] = { instSet.CMP, addrMode.ZeroPage, 3 };
      lookup[0xC6] = { instSet.DEC, addrMode.ZeroPage, 5 };
#ifdef ILLEGAL
      lookup[0xC7] = { instSet.DCP, addrMode.ZeroPage, 5 };
#endif
      lookup[0xC8] = { instSet.INY, addrMode.Implied, 2 };
      lookup[0xC9] = { instSet.CMP, addrMode.Immediate, 2 };
      lookup[0xCA] = { instSet.DEX, addrMode.Implied, 2 };
#ifdef ILLEGAL
      lookup[0xCB] = { instSet.SBX, addrMode.Immediate, 2 };
#endif
      lookup[0xCC] = { instSet.CPY, addrMode.Absolute, 4 };
      lookup[0xCD] = { instSet.CMP, addrMode.Absolute, 4 };
      lookup[0xCE] = { instSet.DEC, addrMode.Absolute, 6 };
#ifdef ILLEGAL
      lookup[0xCF] = { instSet.DCP, addrMode.Absolute, 6 };
#endif
#pragma endregion Instructions 0xC

#pragma region Instructions 0xD
      lookup[0xD0] = { instSet.BNE, addrMode.Relative, 2 };
      lookup[0xD1] = { instSet.CMP, addrMode.IndirectY, 5 };
#ifdef ILLEGAL
      lookup[0xD2] = { instSet.JAM, addrMode.Immediate, 2 };
      lookup[0xD3] = { instSet.DCP, addrMode.IndirectY, 8 };
      lookup[0xD4] = { instSet.DOP, addrMode.ZeroPageX, 4 };
#endif
      lookup[0xD5] = { instSet.CMP, addrMode.ZeroPageX, 4 };
      lookup[0xD6] = { instSet.DEC, addrMode.ZeroPageX, 6 };
#ifdef ILLEGAL
      lookup[0xD7] = { instSet.DCP, addrMode.ZeroPageX, 6 };
#endif
      lookup[0xD8] = { instSet.CLD, addrMode.Implied, 2 };
      lookup[0xD9] = { instSet.CMP, addrMode.AbsoluteY, 4 };
#ifdef ILLEGAL
      lookup[0xDA] = { instSet.NOP, addrMode.Implied, 2 };
      lookup[0xDB] = { instSet.DCP, addrMode.AbsoluteY, 7 };
      lookup[0xDC] = { instSet.TOP, addrMode.AbsoluteX, 4 };
#endif
      lookup[0xDD] = { instSet.CMP, addrMode.AbsoluteX, 4 };
      lookup[0xDE] = { instSet.DEC, addrMode.AbsoluteX, 7 };
#ifdef ILLEGAL
      lookup[0xDF] = { instSet.DCP, addrMode.AbsoluteX, 7 };
#endif
#pragma endregion Instructions 0xD

#pragma region Instructions 0xE
      lookup[0xE0] = { instSet.CPX, addrMode.Immediate, 2 };
      lookup[0xE1] = { instSet.SBC, addrMode.IndirectX, 6 };
#ifdef ILLEGAL
      lookup[0xE2] = { instSet.DOP, addrMode.Immediate, 2 };
      lookup[0xE3] = { instSet.ISC, addrMode.IndirectX, 8 };
#endif
      lookup[0xE4] = { instSet.CPX, addrMode.ZeroPage, 3 };
      lookup[0xE5] = { instSet.SBC, addrMode.ZeroPage, 3 };
      lookup[0xE6] = { instSet.INC, addrMode.ZeroPage, 5 };
#ifdef ILLEGAL
      lookup[0xE7] = { instSet.ISC, addrMode.ZeroPage, 5 };
#endif
      lookup[0xE8] = { instSet.INX, addrMode.Implied, 2 };
      lookup[0xE9] = { instSet.SBC, addrMode.Immediate, 2 };
      lookup[0xEA] = { instSet.NOP, addrMode.Implied, 2 };
#ifdef ILLEGAL
      lookup[0xEB] = { instSet.USBC, addrMode.Immediate, 2 };
#endif
      lookup[0xEC] = { instSet.CPX, addrMode.Absolute, 4 };
      lookup[0xED] = { instSet.SBC, addrMode.Absolute, 4 };
      lookup[0xEE] = { instSet.INC, addrMode.Absolute, 6 };
#ifdef ILLEGAL
      lookup[0xEF] = { instSet.ISC, addrMode.Absolute, 6 };
#endif
#pragma endregion Instructions 0xE

#pragma region Instructions 0xF
      lookup[0xF0] = { instSet.BEQ, addrMode.Relative, 2 };
      lookup[0xF1] = { instSet.SBC, addrMode.IndirectY, 5 };
#ifdef ILLEGAL
      lookup[0xF2] = { instSet.JAM, addrMode.Immediate, 2 };
      lookup[0xF3] = { instSet.ISC, addrMode.IndirectY, 8 };
      lookup[0xF4] = { instSet.DOP, addrMode.ZeroPageX, 4 };
#endif
      lookup[0xF5] = { instSet.SBC, addrMode.ZeroPageX, 4 };
      lookup[0xF6] = { instSet.INC, addrMode.ZeroPageX, 6 };
#ifdef ILLEGAL
      lookup[0xF7] = { instSet.ISC, addrMode.ZeroPageX, 6 };
#endif
      lookup[0xF8] = { instSet.SED, addrMode.Implied, 2 };
      lookup[0xF9] = { instSet.SBC, addrMode.AbsoluteY, 4 };
#ifdef ILLEGAL
      lookup[0xFA] = { instSet.NOP, addrMode.Implied, 2 };
      lookup[0xFB] = { instSet.ISC, addrMode.AbsoluteY, 7 };
      lookup[0xFC] = { instSet.TOP, addrMode.AbsoluteX, 4 };
#endif
      lookup[0xFD] = { instSet.SBC, addrMode.AbsoluteX, 4 };
      lookup[0xFE] = { instSet.INC, addrMode.AbsoluteX, 7 };
#ifdef ILLEGAL
      lookup[0xFF] = { instSet.ISC, addrMode.AbsoluteX, 7 };
#endif
#pragma endregion Instructions 0xF
#pragma endregion Instructions

  };
};
