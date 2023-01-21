//#include "Instructions.hpp"
#include "Executioner.hpp"

namespace CPU
{
  void Executioner::loadInstructions()
  {
#pragma region Instructions
    lookup = {
#pragma region Instructions 0x0
      {0x00, { {&Executioner::BRK, "BRK"}, {&Executioner::IMP, "IMP"}, 7 }},
      {0x01, { {&Executioner::ORA, "ORA"}, {&Executioner::IZX, "IZX"}, 6 }},
#ifdef ILLEGAL
      {0x02, { {&Executioner::JAM, "JAM"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0x03, { {&Executioner::SLO, "SLO"}, {&Executioner::IZX, "IZX"}, 8 }},
      {0x04, { {&Executioner::DOP, "DOP"}, {&Executioner::ZP0, "ZP0"}, 3 }},
#endif
      {0x05, { {&Executioner::ORA, "ORA"}, {&Executioner::ZP0, "ZP0"}, 3 }},
      {0x06, { {&Executioner::ASL, "ASL"}, {&Executioner::ZP0, "ZP0"}, 5 }},
#ifdef ILLEGAL
      {0x07, { {&Executioner::SLO, "SLO"}, {&Executioner::ZP0, "ZP0"}, 5 }},
#endif
      {0x08, { {&Executioner::PHP, "PHP"}, {&Executioner::IMP, "IMP"}, 3 }},
      {0x09, { {&Executioner::ORA, "ORA"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0x0A, { {&Executioner::ASL, "ASL"}, {&Executioner::ACC, "ACC"}, 2 }},
#ifdef ILLEGAL
      {0x0B, { {&Executioner::ANC, "ANC"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0x0C, { {&Executioner::TOP, "TOP"}, {&Executioner::ABS, "ABS"}, 4 }},
#endif
      {0x0D, { {&Executioner::ORA, "ORA"}, {&Executioner::ABS, "ABS"}, 4 }},
      {0x0E, { {&Executioner::ASL, "ASL"}, {&Executioner::ABS, "ABS"}, 6 }},
#ifdef ILLEGAL
      {0x0F, { {&Executioner::SLO, "SLO"}, {&Executioner::ABS, "ABS"}, 6 }},
#endif
#pragma endregion Instructions 0x0

#pragma region Instructions 0x1
      {0x10, { {&Executioner::BPL, "BPL"}, {&Executioner::REL, "REL"}, 2 }},
      {0x11, { {&Executioner::ORA, "ORA"}, {&Executioner::IZY, "IZY"}, 5 }},
#ifdef ILLEGAL
      {0x12, { {&Executioner::JAM, "JAM"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0x13, { {&Executioner::SLO, "SLO"}, {&Executioner::IZY, "IZY"}, 8 }},
      {0x14, { {&Executioner::DOP, "DOP"}, {&Executioner::ZPX, "ZPX"}, 4 }},
#endif
      {0x15, { {&Executioner::ORA, "ORA"}, {&Executioner::ZPX, "ZPX"}, 4 }},
      {0x16, { {&Executioner::ASL, "ASL"}, {&Executioner::ZPX, "ZPX"}, 6 }},
#ifdef ILLEGAL
      {0x17, { {&Executioner::SLO, "SLO"}, {&Executioner::ZPX, "ZPX"}, 6 }},
#endif
      {0x18, { {&Executioner::CLC, "CLC"}, {&Executioner::IMP, "IMP"}, 2 }},
      {0x19, { {&Executioner::ORA, "ORA"}, {&Executioner::ABY, "ABY"}, 4 }},
#ifdef ILLEGAL
      {0x1A, { {&Executioner::NOP, "NOP"}, {&Executioner::IMP, "IMP"}, 2 }},
      {0x1B, { {&Executioner::SLO, "SLO"}, {&Executioner::ABY, "ABY"}, 7 }},
      {0x1C, { {&Executioner::TOP, "TOP"}, {&Executioner::ABX, "ABX"}, 4 }},
#endif
      {0x1D, { {&Executioner::ORA, "ORA"}, {&Executioner::ABX, "ABX"}, 4 }},
      {0x1E, { {&Executioner::ASL, "ASL"}, {&Executioner::ABX, "ABX"}, 7 }},
#ifdef ILLEGAL
      {0x1F, { {&Executioner::SLO, "SLO"}, {&Executioner::ABX, "ABX"}, 7 }},
#endif
#pragma endregion Instructions 0x1

#pragma region Instructions 0x2
      {0x20, { {&Executioner::JSR, "JSR"}, {&Executioner::ABS, "ABS"}, 6 }},
      {0x21, { {&Executioner::AND, "AND"}, {&Executioner::IZX, "IZX"}, 6 }},
#ifdef ILLEGAL
      {0x22, { {&Executioner::JAM, "JAM"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0x23, { {&Executioner::RRA, "RRA"}, {&Executioner::IZX, "IZX"}, 8 }},
#endif
      {0x24, { {&Executioner::BIT, "BIT"}, {&Executioner::ZP0, "ZP0"}, 3 }},
      {0x25, { {&Executioner::AND, "AND"}, {&Executioner::ZP0, "ZP0"}, 3 }},
      {0x26, { {&Executioner::ROL, "ROL"}, {&Executioner::ZP0, "ZP0"}, 5 }},
#ifdef ILLEGAL
      {0x27, { {&Executioner::RRA, "RRA"}, {&Executioner::ZP0, "ZP0"}, 5 }},
#endif
      {0x28, { {&Executioner::PLP, "PLP"}, {&Executioner::IMP, "IMP"}, 4 }},
      {0x29, { {&Executioner::AND, "AND"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0x2A, { {&Executioner::ROL, "ROL"}, {&Executioner::ACC, "ACC"}, 2 }},
#ifdef ILLEGAL
      {0x2B, { {&Executioner::ANC2, "ANC2"}, {&Executioner::IMM, "IMM"}, 2 }},
#endif
      {0x2C, { {&Executioner::BIT, "BIT"}, {&Executioner::ABS, "ABS"}, 4 }},
      {0x2D, { {&Executioner::AND, "AND"}, {&Executioner::ABS, "ABS"}, 4 }},
      {0x2E, { {&Executioner::ROL, "ROL"}, {&Executioner::ABS, "ABS"}, 6 }},
#ifdef ILLEGAL
      {0x2F, { {&Executioner::RRA, "RRA"}, {&Executioner::ABS, "ABS"}, 6 }},
#endif
#pragma endregion Instructions 0x2

#pragma region Instructions 0x3
      {0x30, { {&Executioner::BMI, "BMI"}, {&Executioner::REL, "REL"}, 2 }},
      {0x31, { {&Executioner::AND, "AND"}, {&Executioner::IZY, "IZY"}, 5 }},
#ifdef ILLEGAL
      {0x32, { {&Executioner::JAM, "JAM"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0x33, { {&Executioner::RRA, "RRA"}, {&Executioner::IZY, "IZY"}, 8 }},
      {0x34, { {&Executioner::DOP, "DOP"}, {&Executioner::ZPX, "ZPX"}, 4 }},
#endif
      {0x35, { {&Executioner::AND, "AND"}, {&Executioner::ZPX, "ZPX"}, 4 }},
      {0x36, { {&Executioner::ROL, "ROL"}, {&Executioner::ZPX, "ZPX"}, 6 }},
#ifdef ILLEGAL
      {0x37, { {&Executioner::RRA, "RRA"}, {&Executioner::ZPX, "ZPX"}, 6 }},
#endif
      {0x38, { {&Executioner::SEC, "SEC"}, {&Executioner::IMP, "IMP"}, 2 }},
      {0x39, { {&Executioner::AND, "AND"}, {&Executioner::ABY, "ABY"}, 4 }},
#ifdef ILLEGAL
      {0x3A, { {&Executioner::NOP, "NOP"}, {&Executioner::IMP, "IMP"}, 2 }},
      {0x3B, { {&Executioner::RRA, "RRA"}, {&Executioner::ABY, "ABY"}, 7 }},
      {0x3C, { {&Executioner::TOP, "TOP"}, {&Executioner::ABX, "ABX"}, 4 }},
#endif
      {0x3D, { {&Executioner::AND, "AND"}, {&Executioner::ABX, "ABX"}, 4 }},
      {0x3E, { {&Executioner::ROL, "ROL"}, {&Executioner::ABX, "ABX"}, 7 }},
#ifdef ILLEGAL
      {0x3F, { {&Executioner::RRA, "RRA"}, {&Executioner::ABX, "ABX"}, 7 }},
#endif
#pragma endregion Instructions 0x3

#pragma region Instructions 0x4
      {0x40, { {&Executioner::RTI, "RTI"}, {&Executioner::IMP, "IMP"}, 6 }},
      {0x41, { {&Executioner::EOR, "EOR"}, {&Executioner::IZX, "IZX"}, 6 }},
#ifdef ILLEGAL
      {0x42, { {&Executioner::JAM, "JAM"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0x43, { {&Executioner::SRE, "SRE"}, {&Executioner::IZX, "IZX"}, 8 }},
      {0x44, { {&Executioner::DOP, "DOP"}, {&Executioner::ZP0, "ZP0"}, 3 }},
#endif
      {0x45, { {&Executioner::EOR, "EOR"}, {&Executioner::ZP0, "ZP0"}, 3 }},
      {0x46, { {&Executioner::LSR, "LSR"}, {&Executioner::ZP0, "ZP0"}, 5 }},
#ifdef ILLEGAL
      {0x47, { {&Executioner::SRE, "SRE"}, {&Executioner::ZP0, "ZP0"}, 5 }},
#endif
      {0x48, { {&Executioner::PHA, "PHA"}, {&Executioner::IMP, "IMP"}, 3 }},
      {0x49, { {&Executioner::EOR, "EOR"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0x4A, { {&Executioner::LSR, "LSR"}, {&Executioner::ACC, "ACC"}, 2 }},
#ifdef ILLEGAL
      {0x4B, { {&Executioner::ALR, "ALR"}, {&Executioner::IMM, "IMM"}, 2 }},
#endif
      {0x4C, { {&Executioner::JMP, "JMP"}, {&Executioner::ABS, "ABS"}, 3 }},
      {0x4D, { {&Executioner::EOR, "EOR"}, {&Executioner::ABS, "ABS"}, 4 }},
      {0x4E, { {&Executioner::LSR, "LSR"}, {&Executioner::ABS, "ABS"}, 6 }},
#ifdef ILLEGAL
      {0x4F, { {&Executioner::SRE, "SRE"}, {&Executioner::ABS, "ABS"}, 6 }},
#endif
#pragma endregion Instructions 0x4

#pragma region Instructions 0x5
      {0x50, { {&Executioner::BVC, "BVC"}, {&Executioner::REL, "REL"}, 2 }},
      {0x51, { {&Executioner::EOR, "EOR"}, {&Executioner::IZY, "IZY"}, 5 }},
#ifdef ILLEGAL
      {0x52, { {&Executioner::JAM, "JAM"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0x53, { {&Executioner::SRE, "SRE"}, {&Executioner::IZY, "IZY"}, 8 }},
      {0x54, { {&Executioner::DOP, "DOP"}, {&Executioner::ZPX, "ZPX"}, 4 }},
#endif
      {0x55, { {&Executioner::EOR, "EOR"}, {&Executioner::ZPX, "ZPX"}, 4 }},
      {0x56, { {&Executioner::LSR, "LSR"}, {&Executioner::ZPX, "ZPX"}, 6 }},
#ifdef ILLEGAL
      {0x57, { {&Executioner::SRE, "SRE"}, {&Executioner::ZPX, "ZPX"}, 6 }},
#endif
      {0x58, { {&Executioner::CLI, "CLI"}, {&Executioner::IMP, "IMP"}, 2 }},
      {0x59, { {&Executioner::EOR, "EOR"}, {&Executioner::ABY, "ABY"}, 4 }},
#ifdef ILLEGAL
      {0x5A, { {&Executioner::NOP, "NOP"}, {&Executioner::IMP, "IMP"}, 2 }},
      {0x5B, { {&Executioner::SRE, "SRE"}, {&Executioner::ABY, "ABY"}, 7 }},
      {0x5C, { {&Executioner::TOP, "TOP"}, {&Executioner::ABX, "ABX"}, 4 }},
#endif
      {0x5D, { {&Executioner::EOR, "EOR"}, {&Executioner::ABX, "ABX"}, 4 }},
      {0x5E, { {&Executioner::LSR, "LSR"}, {&Executioner::ABX, "ABX"}, 7 }},
#ifdef ILLEGAL
      {0x5F, { {&Executioner::SRE, "SRE"}, {&Executioner::ABX, "ABX"}, 7 }},
#endif
#pragma endregion Instructions 0x5

#pragma region Instructions 0x6
      {0x60, { {&Executioner::RTS, "RTS"}, {&Executioner::IMP, "IMP"}, 6 }},
      {0x61, { {&Executioner::ADC, "ADC"}, {&Executioner::IZX, "IZX"}, 6 }},
#ifdef ILLEGAL
      {0x62, { {&Executioner::JAM, "JAM"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0x63, { {&Executioner::RLA, "RLA"}, {&Executioner::IZX, "IZX"}, 8 }},
      {0x64, { {&Executioner::DOP, "DOP"}, {&Executioner::ZP0, "ZP0"}, 3 }},
#endif
      {0x65, { {&Executioner::ADC, "ADC"}, {&Executioner::ZP0, "ZP0"}, 3 }},
      {0x66, { {&Executioner::ROR, "ROR"}, {&Executioner::ZP0, "ZP0"}, 5 }},
#ifdef ILLEGAL
      {0x67, { {&Executioner::RLA, "RLA"}, {&Executioner::ZP0, "ZP0"}, 5 }},
#endif
      {0x68, { {&Executioner::PLA, "PLA"}, {&Executioner::IMP, "IMP"}, 4 }},
      {0x69, { {&Executioner::ADC, "ADC"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0x6A, { {&Executioner::ROR, "ROR"}, {&Executioner::ACC, "ACC"}, 2 }},
#ifdef ILLEGAL
      {0x6B, { {&Executioner::ARR, "ARR"}, {&Executioner::IMM, "IMM"}, 2 }},
#endif
      {0x6C, { {&Executioner::JMP, "JMP"}, {&Executioner::IND, "IND"}, 5 }},
      {0x6D, { {&Executioner::ADC, "ADC"}, {&Executioner::ABS, "ABS"}, 4 }},
      {0x6E, { {&Executioner::ROR, "ROR"}, {&Executioner::ABS, "ABS"}, 6 }},
#ifdef ILLEGAL
      {0x6F, { {&Executioner::RLA, "RLA"}, {&Executioner::ABS, "ABS"}, 6 }},
#endif
#pragma endregion Instructions 0x6

#pragma region Instructions 0x7
      {0x70, { {&Executioner::BVS, "BVS"}, {&Executioner::REL, "REL"}, 2 }},
      {0x71, { {&Executioner::ADC, "ADC"}, {&Executioner::IZY, "IZY"}, 5 }},
#ifdef ILLEGAL
      {0x72, { {&Executioner::JAM, "JAM"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0x73, { {&Executioner::RLA, "RLA"}, {&Executioner::IZY, "IZY"}, 8 }},
      {0x74, { {&Executioner::DOP, "DOP"}, {&Executioner::ZPX, "ZPX"}, 4 }},
#endif
      {0x75, { {&Executioner::ADC, "ADC"}, {&Executioner::ZPX, "ZPX"}, 4 }},
      {0x76, { {&Executioner::ROR, "ROR"}, {&Executioner::ZPX, "ZPX"}, 6 }},
#ifdef ILLEGAL
      {0x77, { {&Executioner::RLA, "RLA"}, {&Executioner::ZPX, "ZPX"}, 6 }},
#endif
      {0x78, { {&Executioner::SEI, "SEI"}, {&Executioner::IMP, "IMP"}, 2 }},
      {0x79, { {&Executioner::ADC, "ADC"}, {&Executioner::ABY, "ABY"}, 4 }},
#ifdef ILLEGAL
      {0x7A, { {&Executioner::NOP, "NOP"}, {&Executioner::IMP, "IMP"}, 2 }},
      {0x7B, { {&Executioner::RLA, "RLA"}, {&Executioner::ABY, "ABY"}, 7 }},
      {0x7C, { {&Executioner::TOP, "TOP"}, {&Executioner::ABX, "ABX"}, 4 }},
#endif
      {0x7D, { {&Executioner::ADC, "ADC"}, {&Executioner::ABX, "ABX"}, 4 }},
      {0x7E, { {&Executioner::ROR, "ROR"}, {&Executioner::ABX, "ABX"}, 7 }},
#ifdef ILLEGAL
      {0x7F, { {&Executioner::RLA, "RLA"}, {&Executioner::ABX, "ABX"}, 7 }},
#endif
#pragma endregion Instructions 0x7

#pragma region Instructions 0x8
#ifdef ILLEGAL
      {0x80, { {&Executioner::DOP, "DOP"}, {&Executioner::IMM, "IMM"}, 2 }},
#endif
      {0x81, { {&Executioner::STA, "STA"}, {&Executioner::IZX, "IZX"}, 6 }},
#ifdef ILLEGAL
      {0x82, { {&Executioner::DOP, "DOP"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0x83, { {&Executioner::SAX, "SAX"}, {&Executioner::IZX, "IZX"}, 6 }},
#endif
      {0x84, { {&Executioner::STY, "STY"}, {&Executioner::ZP0, "ZP0"}, 3 }},
      {0x85, { {&Executioner::STA, "STA"}, {&Executioner::ZP0, "ZP0"}, 3 }},
      {0x86, { {&Executioner::STX, "STX"}, {&Executioner::ZP0, "ZP0"}, 3 }},
#ifdef ILLEGAL
      {0x87, { {&Executioner::SAX, "SAX"}, {&Executioner::ZP0, "ZP0"}, 3 }},
#endif
      {0x88, { {&Executioner::DEY, "DEY"}, {&Executioner::IMP, "IMP"}, 2 }},
#ifdef ILLEGAL

      {0x89, { {&Executioner::DOP, "DOP"}, {&Executioner::IMM, "IMM"}, 2 }},
#endif
      {0x8A, { {&Executioner::TXA, "TXA"}, {&Executioner::IMP, "IMP"}, 2 }},
#ifdef ILLEGAL
      {0x8B, { {&Executioner::ANE, "ANE"}, {&Executioner::IMM, "IMM"}, 2 }},
#endif
      {0x8C, { {&Executioner::STY, "STY"}, {&Executioner::ABS, "ABS"}, 4 }},
      {0x8D, { {&Executioner::STA, "STA"}, {&Executioner::ABS, "ABS"}, 4 }},
      {0x8E, { {&Executioner::STX, "STX"}, {&Executioner::ABS, "ABS"}, 4 }},
#ifdef ILLEGAL
      {0x8F, { {&Executioner::SAX, "SAX"}, {&Executioner::ABS, "ABS"}, 4 }},
#endif
#pragma endregion Instructions 0x8

#pragma region Instructions 0x9
      {0x90, { {&Executioner::BCC, "BCC"}, {&Executioner::REL, "REL"}, 2 }},
      {0x91, { {&Executioner::STA, "STA"}, {&Executioner::IZY, "IZY"}, 6 }},
#ifdef ILLEGAL
      {0x92, { {&Executioner::JAM, "JAM"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0x93, { {&Executioner::SHA, "SHA"}, {&Executioner::IZY, "IZY"}, 6 }},
#endif
      {0x94, { {&Executioner::STY, "STY"}, {&Executioner::ZPX, "ZPX"}, 4 }},
      {0x95, { {&Executioner::STA, "STA"}, {&Executioner::ZPX, "ZPX"}, 4 }},
      {0x96, { {&Executioner::STX, "STX"}, {&Executioner::ZPY, "ZPY"}, 4 }},
#ifdef ILLEGAL
      {0x97, { {&Executioner::SAX, "SAX"}, {&Executioner::ZPY, "ZPY"}, 4 }},
#endif
      {0x98, { {&Executioner::TYA, "TYA"}, {&Executioner::IMP, "IMP"}, 2 }},
      {0x99, { {&Executioner::STA, "STA"}, {&Executioner::ABY, "ABY"}, 5 }},
      {0x9A, { {&Executioner::TXS, "TXS"}, {&Executioner::IMP, "IMP"}, 2 }},
#ifdef ILLEGAL
      {0x9B, { {&Executioner::TAS, "TAS"}, {&Executioner::ABY, "ABY"}, 5 }},
      {0x9C, { {&Executioner::SHY, "SHY"}, {&Executioner::ABX, "ABX"}, 5 }},
#endif
      {0x9D, { {&Executioner::STA, "STA"}, {&Executioner::ABX, "ABX"}, 5 }},
#ifdef ILLEGAL
      {0x9E, { {&Executioner::SHX, "SHX"}, {&Executioner::ABY, "ABY"}, 5 }},
      {0x9F, { {&Executioner::SHA, "SHA"}, {&Executioner::ABY, "ABY"}, 5 }},
#endif
#pragma endregion Instructions 0x9

#pragma region Instructions 0xA
      {0xA0, { {&Executioner::LDY, "LDY"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0xA1, { {&Executioner::LDA, "LDA"}, {&Executioner::IZX, "IZX"}, 6 }},
      {0xA2, { {&Executioner::LDX, "LDX"}, {&Executioner::IMM, "IMM"}, 2 }},
#ifdef ILLEGAL
      {0xA3, { {&Executioner::LAX, "LAX"}, {&Executioner::IZX, "IZX"}, 6 }},
#endif
      {0xA4, { {&Executioner::LDY, "LDY"}, {&Executioner::ZP0, "ZP0"}, 3 }},
      {0xA5, { {&Executioner::LDA, "LDA"}, {&Executioner::ZP0, "ZP0"}, 3 }},
      {0xA6, { {&Executioner::LDX, "LDX"}, {&Executioner::ZP0, "ZP0"}, 3 }},
#ifdef ILLEGAL
      {0xA7, { {&Executioner::LAX, "LAX"}, {&Executioner::ZP0, "ZP0"}, 3 }},
#endif
      {0xA8, { {&Executioner::TAY, "TAY"}, {&Executioner::IMP, "IMP"}, 2 }},
      {0xA9, { {&Executioner::LDA, "LDA"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0xAA, { {&Executioner::TAX, "TAX"}, {&Executioner::IMP, "IMP"}, 2 }},
#ifdef ILLEGAL
      {0xAB, { {&Executioner::LXA, "LXA"}, {&Executioner::IMM, "IMM"}, 2 }},
#endif
      {0xAC, { {&Executioner::LDY, "LDY"}, {&Executioner::ABS, "ABS"}, 4 }},
      {0xAD, { {&Executioner::LDA, "LDA"}, {&Executioner::ABS, "ABS"}, 4 }},
      {0xAE, { {&Executioner::LDX, "LDX"}, {&Executioner::ABS, "ABS"}, 4 }},
#ifdef ILLEGAL
      {0xAF, { {&Executioner::LAX, "LAX"}, {&Executioner::ABS, "ABS"}, 4 }},
#endif
#pragma endregion Instructions 0xA

#pragma region Instructions 0xB
      {0xB0, { {&Executioner::BCS, "BCS"}, {&Executioner::REL, "REL"}, 2 }},
      {0xB1, { {&Executioner::LDA, "LDA"}, {&Executioner::IZY, "IZY"}, 5 }},
#ifdef ILLEGAL
      {0xB2, { {&Executioner::JAM, "JAM"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0xB3, { {&Executioner::LAX, "LAX"}, {&Executioner::IZY, "IZY"}, 5 }},
#endif
      {0xB4, { {&Executioner::LDY, "LDY"}, {&Executioner::ZPX, "ZPX"}, 4 }},
      {0xB5, { {&Executioner::LDA, "LDA"}, {&Executioner::ZPX, "ZPX"}, 4 }},
      {0xB6, { {&Executioner::LDX, "LDX"}, {&Executioner::ZPY, "ZPY"}, 4 }},
#ifdef ILLEGAL
      {0xB7, { {&Executioner::LAX, "LAX"}, {&Executioner::ZPX, "ZPX"}, 4 }},
#endif
      {0xB8, { {&Executioner::CLV, "CLV"}, {&Executioner::IMP, "IMP"}, 2 }},
      {0xB9, { {&Executioner::LDA, "LDA"}, {&Executioner::ABY, "ABY"}, 4 }},
      {0xBA, { {&Executioner::TSX, "TSX"}, {&Executioner::IMP, "IMP"}, 2 }},
#ifdef ILLEGAL
      {0xBB, { {&Executioner::LAS, "LAS"}, {&Executioner::ABY, "ABY"}, 4 }},
#endif
      {0xBC, { {&Executioner::LDY, "LDY"}, {&Executioner::ABX, "ABX"}, 4 }},
      {0xBD, { {&Executioner::LDA, "LDA"}, {&Executioner::ABX, "ABX"}, 4 }},
      {0xBE, { {&Executioner::LDX, "LDX"}, {&Executioner::ABY, "ABY"}, 4 }},
#ifdef ILLEGAL
      {0xBF, { {&Executioner::LAX, "LAX"}, {&Executioner::ABY, "ABY"}, 4 }},
#endif
#pragma endregion Instructions 0xB

#pragma region Instructions 0xC
      {0xC0, { {&Executioner::CPY, "CPY"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0xC1, { {&Executioner::CMP, "CMP"}, {&Executioner::IZX, "IZX"}, 6 }},
#ifdef ILLEGAL
      {0xC2, { {&Executioner::DOP, "DOP"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0xC3, { {&Executioner::DCP, "DCP"}, {&Executioner::IZX, "IZX"}, 8 }},
#endif
      {0xC4, { {&Executioner::CPY, "CPY"}, {&Executioner::ZP0, "ZP0"}, 3 }},
      {0xC5, { {&Executioner::CMP, "CMP"}, {&Executioner::ZP0, "ZP0"}, 3 }},
      {0xC6, { {&Executioner::DEC, "DEC"}, {&Executioner::ZP0, "ZP0"}, 5 }},
#ifdef ILLEGAL
      {0xC7, { {&Executioner::DCP, "DCP"}, {&Executioner::ZP0, "ZP0"}, 5 }},
#endif
      {0xC8, { {&Executioner::INY, "INY"}, {&Executioner::IMP, "IMP"}, 2 }},
      {0xC9, { {&Executioner::CMP, "CMP"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0xCA, { {&Executioner::DEX, "DEX"}, {&Executioner::IMP, "IMP"}, 2 }},
#ifdef ILLEGAL
      {0xCB, { {&Executioner::SBX, "SBX"}, {&Executioner::IMM, "IMM"}, 2 }},
#endif
      {0xCC, { {&Executioner::CPY, "CPY"}, {&Executioner::ABS, "ABS"}, 4 }},
      {0xCD, { {&Executioner::CMP, "CMP"}, {&Executioner::ABS, "ABS"}, 4 }},
      {0xCE, { {&Executioner::DEC, "DEC"}, {&Executioner::ABS, "ABS"}, 6 }},
#ifdef ILLEGAL
      {0xCF, { {&Executioner::DCP, "DCP"}, {&Executioner::ABS, "ABS"}, 6 }},
#endif
#pragma endregion Instructions 0xC

#pragma region Instructions 0xD
      {0xD0, { {&Executioner::BNE, "BNE"}, {&Executioner::REL, "REL"}, 2 }},
      {0xD1, { {&Executioner::CMP, "CMP"}, {&Executioner::IZY, "IZY"}, 5 }},
#ifdef ILLEGAL
      {0xD2, { {&Executioner::JAM, "JAM"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0xD3, { {&Executioner::DCP, "DCP"}, {&Executioner::IZY, "IZY"}, 8 }},
      {0xD4, { {&Executioner::DOP, "DOP"}, {&Executioner::ZPX, "ZPX"}, 4 }},
#endif
      {0xD5, { {&Executioner::CMP, "CMP"}, {&Executioner::ZPX, "ZPX"}, 4 }},
      {0xD6, { {&Executioner::DEC, "DEC"}, {&Executioner::ZPX, "ZPX"}, 6 }},
#ifdef ILLEGAL
      {0xD7, { {&Executioner::DCP, "DCP"}, {&Executioner::ZPX, "ZPX"}, 6 }},
#endif
      {0xD8, { {&Executioner::CLD, "CLD"}, {&Executioner::IMP, "IMP"}, 2 }},
      {0xD9, { {&Executioner::CMP, "CMP"}, {&Executioner::ABY, "ABY"}, 4 }},
#ifdef ILLEGAL
      {0xDA, { {&Executioner::NOP, "NOP"}, {&Executioner::IMP, "IMP"}, 2 }},
      {0xDB, { {&Executioner::DCP, "DCP"}, {&Executioner::ABY, "ABY"}, 7 }},
      {0xDC, { {&Executioner::TOP, "TOP"}, {&Executioner::ABX, "ABX"}, 4 }},
#endif
      {0xDD, { {&Executioner::CMP, "CMP"}, {&Executioner::ABX, "ABX"}, 4 }},
      {0xDE, { {&Executioner::DEC, "DEC"}, {&Executioner::ABX, "ABX"}, 7 }},
#ifdef ILLEGAL
      {0xDF, { {&Executioner::DCP, "DCP"}, {&Executioner::ABX, "ABX"}, 7 }},
#endif
#pragma endregion Instructions 0xD

#pragma region Instructions 0xE
      {0xE0, { {&Executioner::CPX, "CPX"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0xE1, { {&Executioner::SBC, "SBC"}, {&Executioner::IZX, "IZX"}, 6 }},
#ifdef ILLEGAL
      {0xE2, { {&Executioner::DOP, "DOP"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0xE3, { {&Executioner::ISC, "ISC"}, {&Executioner::IZX, "IZX"}, 8 }},
#endif
      {0xE4, { {&Executioner::CPX, "CPX"}, {&Executioner::ZP0, "ZP0"}, 3 }},
      {0xE5, { {&Executioner::SBC, "SBC"}, {&Executioner::ZP0, "ZP0"}, 3 }},
      {0xE6, { {&Executioner::INC, "INC"}, {&Executioner::ZP0, "ZP0"}, 5 }},
#ifdef ILLEGAL
      {0xE7, { {&Executioner::ISC, "ISC"}, {&Executioner::ZP0, "ZP0"}, 5 }},
#endif
      {0xE8, { {&Executioner::INX, "INX"}, {&Executioner::IMP, "IMP"}, 2 }},
      {0xE9, { {&Executioner::SBC, "SBC"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0xEA, { {&Executioner::NOP, "NOP"}, {&Executioner::IMP, "IMP"}, 2 }},
#ifdef ILLEGAL
      {0xEB, { {&Executioner::USBC, "USBC"}, {&Executioner::IMM, "IMM"}, 2 }},
#endif
      {0xEC, { {&Executioner::CPX, "CPX"}, {&Executioner::ABS, "ABS"}, 4 }},
      {0xED, { {&Executioner::SBC, "SBC"}, {&Executioner::ABS, "ABS"}, 4 }},
      {0xEE, { {&Executioner::INC, "INC"}, {&Executioner::ABS, "ABS"}, 6 }},
#ifdef ILLEGAL
      {0xEF, { {&Executioner::ISC, "ISC"}, {&Executioner::ABS, "ABS"}, 6 }},
#endif
#pragma endregion Instructions 0xE

#pragma region Instructions 0xF
      {0xF0, { {&Executioner::BEQ, "BEQ"}, {&Executioner::REL, "REL"}, 2 }},
      {0xF1, { {&Executioner::SBC, "SBC"}, {&Executioner::IZY, "IZY"}, 5 }},
#ifdef ILLEGAL
      {0xF2, { {&Executioner::JAM, "JAM"}, {&Executioner::IMM, "IMM"}, 2 }},
      {0xF3, { {&Executioner::ISC, "ISC"}, {&Executioner::IZY, "IZY"}, 8 }},
      {0xF4, { {&Executioner::DOP, "DOP"}, {&Executioner::ZPX, "ZPX"}, 4 }},
#endif
      {0xF5, { {&Executioner::SBC, "SBC"}, {&Executioner::ZPX, "ZPX"}, 4 }},
      {0xF6, { {&Executioner::INC, "INC"}, {&Executioner::ZPX, "ZPX"}, 6 }},
#ifdef ILLEGAL
      {0xF7, { {&Executioner::ISC, "ISC"}, {&Executioner::ZPX, "ZPX"}, 6 }},
#endif
      {0xF8, { {&Executioner::SED, "SED"}, {&Executioner::IMP, "IMP"}, 2 }},
      {0xF9, { {&Executioner::SBC, "SBC"}, {&Executioner::ABY, "ABY"}, 4 }},
#ifdef ILLEGAL
      {0xFA, { {&Executioner::NOP, "NOP"}, {&Executioner::IMP, "IMP"}, 2 }},
      {0xFB, { {&Executioner::ISC, "ISC"}, {&Executioner::ABY, "ABY"}, 7 }},
      {0xFC, { {&Executioner::TOP, "TOP"}, {&Executioner::ABX, "ABX"}, 4 }},
#endif
      {0xFD, { {&Executioner::SBC, "SBC"}, {&Executioner::ABX, "ABX"}, 4 }},
      {0xFE, { {&Executioner::INC, "INC"}, {&Executioner::ABX, "ABX"}, 7 }},
#ifdef ILLEGAL
      {0xFF, { {&Executioner::ISC, "ISC"}, {&Executioner::ABX, "ABX"}, 7 }},
#endif
#pragma endregion Instructions 0xF
    };
#pragma endregion Instructions
  };
};
