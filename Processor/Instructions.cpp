//#include "Instructions.hpp"
#include "Executioner.hpp"
#include <cstdint>
#include <string>


namespace CPU
{
  void Executioner::loadInstructions()
  {
#pragma region Instructions
  lookup = {
#pragma region Instructions 0x0
      {0x00, { {&Executioner::opBRK, "BRK"}, {&Executioner::addrIMP, "IMP"}, 7 }},
      {0x01, { {&Executioner::opORA, "ORA"}, {&Executioner::addrIZX, "IZX"}, 6 }},
#if defined(ILLEGAL)
      {0x02, { {&Executioner::opJAM, "JAM"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0x03, { {&Executioner::opSLO, "SLO"}, {&Executioner::addrIZX, "IZX"}, 8 }},
      {0x04, { {&Executioner::opDOP, "DOP"}, {&Executioner::addrZP0, "ZP0"}, 3 }},
#endif
      {0x05, { {&Executioner::opORA, "ORA"}, {&Executioner::addrZP0, "ZP0"}, 3 }},
      {0x06, { {&Executioner::opASL, "ASL"}, {&Executioner::addrZP0, "ZP0"}, 5 }},
#if defined(ILLEGAL)
      {0x07, { {&Executioner::opSLO, "SLO"}, {&Executioner::addrZP0, "ZP0"}, 5 }},
#endif
      {0x08, { {&Executioner::opPHP, "PHP"}, {&Executioner::addrIMP, "IMP"}, 3 }},
      {0x09, { {&Executioner::opORA, "ORA"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0x0A, { {&Executioner::opASL, "ASL"}, {&Executioner::addrACC, "ACC"}, 2 }},
#if defined(ILLEGAL)
      {0x0B, { {&Executioner::opANC, "ANC"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0x0C, { {&Executioner::opTOP, "TOP"}, {&Executioner::addrABS, "ABS"}, 4 }},
#endif
      {0x0D, { {&Executioner::opORA, "ORA"}, {&Executioner::addrABS, "ABS"}, 4 }},
      {0x0E, { {&Executioner::opASL, "ASL"}, {&Executioner::addrABS, "ABS"}, 6 }},
#if defined(ILLEGAL)
      {0x0F, { {&Executioner::opSLO, "SLO"}, {&Executioner::addrABS, "ABS"}, 6 }},
#endif
#pragma endregion Instructions 0x0

#pragma region Instructions 0x1
      {0x10, { {&Executioner::opBPL, "BPL"}, {&Executioner::addrREL, "REL"}, 2 }},
      {0x11, { {&Executioner::opORA, "ORA"}, {&Executioner::addrIZY, "IZY"}, 5 }},
#if defined(ILLEGAL)
      {0x12, { {&Executioner::opJAM, "JAM"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0x13, { {&Executioner::opSLO, "SLO"}, {&Executioner::addrIZY, "IZY"}, 8 }},
      {0x14, { {&Executioner::opDOP, "DOP"}, {&Executioner::addrZPX, "ZPX"}, 4 }},
#endif
      {0x15, { {&Executioner::opORA, "ORA"}, {&Executioner::addrZPX, "ZPX"}, 4 }},
      {0x16, { {&Executioner::opASL, "ASL"}, {&Executioner::addrZPX, "ZPX"}, 6 }},
#if defined(ILLEGAL)
      {0x17, { {&Executioner::opSLO, "SLO"}, {&Executioner::addrZPX, "ZPX"}, 6 }},
#endif
      {0x18, { {&Executioner::opCLC, "CLC"}, {&Executioner::addrIMP, "IMP"}, 2 }},
      {0x19, { {&Executioner::opORA, "ORA"}, {&Executioner::addrABY, "ABY"}, 4 }},
#if defined(ILLEGAL)
      {0x1A, { {&Executioner::opNOP, "NOP"}, {&Executioner::addrIMP, "IMP"}, 2 }},
      {0x1B, { {&Executioner::opSLO, "SLO"}, {&Executioner::addrABY, "ABY"}, 7 }},
      {0x1C, { {&Executioner::opTOP, "TOP"}, {&Executioner::addrABX, "ABX"}, 4 }},
#endif
      {0x1D, { {&Executioner::opORA, "ORA"}, {&Executioner::addrABX, "ABX"}, 4 }},
#if defined(EMULATE65C02)
      {0x1E, { {&Executioner::opASL, "ASL"}, {&Executioner::addrABX, "ABX"}, 6 }},
#else
      {0x1E, { {&Executioner::opASL, "ASL"}, {&Executioner::addrABX, "ABX"}, 7 }},
#endif
#if defined(ILLEGAL)
      {0x1F, { {&Executioner::opSLO, "SLO"}, {&Executioner::addrABX, "ABX"}, 7 }},
#endif
#pragma endregion Instructions 0x1

#pragma region Instructions 0x2
      {0x20, { {&Executioner::opJSR, "JSR"}, {&Executioner::addrABS, "ABS"}, 6 }},
      {0x21, { {&Executioner::opAND, "AND"}, {&Executioner::addrIZX, "IZX"}, 6 }},
#if defined(ILLEGAL)
      {0x22, { {&Executioner::opJAM, "JAM"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0x23, { {&Executioner::opRRA, "RRA"}, {&Executioner::addrIZX, "IZX"}, 8 }},
#endif
      {0x24, { {&Executioner::opBIT, "BIT"}, {&Executioner::addrZP0, "ZP0"}, 3 }},
      {0x25, { {&Executioner::opAND, "AND"}, {&Executioner::addrZP0, "ZP0"}, 3 }},
      {0x26, { {&Executioner::opROL, "ROL"}, {&Executioner::addrZP0, "ZP0"}, 5 }},
#if defined(ILLEGAL)
      {0x27, { {&Executioner::opRRA, "RRA"}, {&Executioner::addrZP0, "ZP0"}, 5 }},
#endif
      {0x28, { {&Executioner::opPLP, "PLP"}, {&Executioner::addrIMP, "IMP"}, 4 }},
      {0x29, { {&Executioner::opAND, "AND"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0x2A, { {&Executioner::opROL, "ROL"}, {&Executioner::addrACC, "ACC"}, 2 }},
#if defined(ILLEGAL)
      {0x2B, { {&Executioner::opANC2, "ANC2"}, {&Executioner::addrIMM, "IMM"}, 2 }},
#endif
      {0x2C, { {&Executioner::opBIT, "BIT"}, {&Executioner::addrABS, "ABS"}, 4 }},
      {0x2D, { {&Executioner::opAND, "AND"}, {&Executioner::addrABS, "ABS"}, 4 }},
      {0x2E, { {&Executioner::opROL, "ROL"}, {&Executioner::addrABS, "ABS"}, 6 }},
#if defined(ILLEGAL)
      {0x2F, { {&Executioner::opRRA, "RRA"}, {&Executioner::addrABS, "ABS"}, 6 }},
#endif
#pragma endregion Instructions 0x2

#pragma region Instructions 0x3
      {0x30, { {&Executioner::opBMI, "BMI"}, {&Executioner::addrREL, "REL"}, 2 }},
      {0x31, { {&Executioner::opAND, "AND"}, {&Executioner::addrIZY, "IZY"}, 5 }},
#if defined(ILLEGAL)
      {0x32, { {&Executioner::opJAM, "JAM"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0x33, { {&Executioner::opRRA, "RRA"}, {&Executioner::addrIZY, "IZY"}, 8 }},
      {0x34, { {&Executioner::opDOP, "DOP"}, {&Executioner::addrZPX, "ZPX"}, 4 }},
#endif
      {0x35, { {&Executioner::opAND, "AND"}, {&Executioner::addrZPX, "ZPX"}, 4 }},
      {0x36, { {&Executioner::opROL, "ROL"}, {&Executioner::addrZPX, "ZPX"}, 6 }},
#if defined(ILLEGAL)
      {0x37, { {&Executioner::opRRA, "RRA"}, {&Executioner::addrZPX, "ZPX"}, 6 }},
#endif
      {0x38, { {&Executioner::opSEC, "SEC"}, {&Executioner::addrIMP, "IMP"}, 2 }},
      {0x39, { {&Executioner::opAND, "AND"}, {&Executioner::addrABY, "ABY"}, 4 }},
#if defined(ILLEGAL)
      {0x3A, { {&Executioner::opNOP, "NOP"}, {&Executioner::addrIMP, "IMP"}, 2 }},
      {0x3B, { {&Executioner::opRRA, "RRA"}, {&Executioner::addrABY, "ABY"}, 7 }},
      {0x3C, { {&Executioner::opTOP, "TOP"}, {&Executioner::addrABX, "ABX"}, 4 }},
#endif
      {0x3D, { {&Executioner::opAND, "AND"}, {&Executioner::addrABX, "ABX"}, 4 }},
#if defined(EMULATE65C02)
      {0x3E, { {&Executioner::opROL, "ROL"}, {&Executioner::addrABX, "ABX"}, 6 }},
#else
      {0x3E, { {&Executioner::opROL, "ROL"}, {&Executioner::addrABX, "ABX"}, 7 }},
#endif
#if defined(ILLEGAL)
      {0x3F, { {&Executioner::opRRA, "RRA"}, {&Executioner::addrABX, "ABX"}, 7 }},
#endif
#pragma endregion Instructions 0x3

#pragma region Instructions 0x4
      {0x40, { {&Executioner::opRTI, "RTI"}, {&Executioner::addrIMP, "IMP"}, 6 }},
      {0x41, { {&Executioner::opEOR, "EOR"}, {&Executioner::addrIZX, "IZX"}, 6 }},
#if defined(ILLEGAL)
      {0x42, { {&Executioner::opJAM, "JAM"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0x43, { {&Executioner::opSRE, "SRE"}, {&Executioner::addrIZX, "IZX"}, 8 }},
      {0x44, { {&Executioner::opDOP, "DOP"}, {&Executioner::addrZP0, "ZP0"}, 3 }},
#endif
      {0x45, { {&Executioner::opEOR, "EOR"}, {&Executioner::addrZP0, "ZP0"}, 3 }},
      {0x46, { {&Executioner::opLSR, "LSR"}, {&Executioner::addrZP0, "ZP0"}, 5 }},
#if defined(ILLEGAL)
      {0x47, { {&Executioner::opSRE, "SRE"}, {&Executioner::addrZP0, "ZP0"}, 5 }},
#endif
      {0x48, { {&Executioner::opPHA, "PHA"}, {&Executioner::addrIMP, "IMP"}, 3 }},
      {0x49, { {&Executioner::opEOR, "EOR"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0x4A, { {&Executioner::opLSR, "LSR"}, {&Executioner::addrACC, "ACC"}, 2 }},
#if defined(ILLEGAL)
      {0x4B, { {&Executioner::opALR, "ALR"}, {&Executioner::addrIMM, "IMM"}, 2 }},
#endif
      {0x4C, { {&Executioner::opJMP, "JMP"}, {&Executioner::addrABS, "ABS"}, 3 }},
      {0x4D, { {&Executioner::opEOR, "EOR"}, {&Executioner::addrABS, "ABS"}, 4 }},
      {0x4E, { {&Executioner::opLSR, "LSR"}, {&Executioner::addrABS, "ABS"}, 6 }},
#if defined(ILLEGAL)
      {0x4F, { {&Executioner::opSRE, "SRE"}, {&Executioner::addrABS, "ABS"}, 6 }},
#endif
#pragma endregion Instructions 0x4

#pragma region Instructions 0x5
      {0x50, { {&Executioner::opBVC, "BVC"}, {&Executioner::addrREL, "REL"}, 2 }},
      {0x51, { {&Executioner::opEOR, "EOR"}, {&Executioner::addrIZY, "IZY"}, 5 }},
#if defined(ILLEGAL)
      {0x52, { {&Executioner::opJAM, "JAM"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0x53, { {&Executioner::opSRE, "SRE"}, {&Executioner::addrIZY, "IZY"}, 8 }},
      {0x54, { {&Executioner::opDOP, "DOP"}, {&Executioner::addrZPX, "ZPX"}, 4 }},
#endif
      {0x55, { {&Executioner::opEOR, "EOR"}, {&Executioner::addrZPX, "ZPX"}, 4 }},
      {0x56, { {&Executioner::opLSR, "LSR"}, {&Executioner::addrZPX, "ZPX"}, 6 }},
#if defined(ILLEGAL)
      {0x57, { {&Executioner::opSRE, "SRE"}, {&Executioner::addrZPX, "ZPX"}, 6 }},
#endif
      {0x58, { {&Executioner::opCLI, "CLI"}, {&Executioner::addrIMP, "IMP"}, 2 }},
      {0x59, { {&Executioner::opEOR, "EOR"}, {&Executioner::addrABY, "ABY"}, 4 }},
#if defined(ILLEGAL)
      {0x5A, { {&Executioner::opNOP, "NOP"}, {&Executioner::addrIMP, "IMP"}, 2 }},
      {0x5B, { {&Executioner::opSRE, "SRE"}, {&Executioner::addrABY, "ABY"}, 7 }},
      {0x5C, { {&Executioner::opTOP, "TOP"}, {&Executioner::addrABX, "ABX"}, 4 }},
#endif
      {0x5D, { {&Executioner::opEOR, "EOR"}, {&Executioner::addrABX, "ABX"}, 4 }},
#if defined(EMULATE65C02)
      {0x5E, { {&Executioner::opLSR, "LSR"}, {&Executioner::addrABX, "ABX"}, 6 }},
#else
      {0x5E, { {&Executioner::opLSR, "LSR"}, {&Executioner::addrABX, "ABX"}, 7 }},
#endif
#if defined(ILLEGAL)
      {0x5F, { {&Executioner::opSRE, "SRE"}, {&Executioner::addrABX, "ABX"}, 7 }},
#endif
#pragma endregion Instructions 0x5

#pragma region Instructions 0x6
      {0x60, { {&Executioner::opRTS, "RTS"}, {&Executioner::addrIMP, "IMP"}, 6 }},
      {0x61, { {&Executioner::opADC, "ADC"}, {&Executioner::addrIZX, "IZX"}, 6 }},
#if defined(ILLEGAL)
      {0x62, { {&Executioner::opJAM, "JAM"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0x63, { {&Executioner::opRLA, "RLA"}, {&Executioner::addrIZX, "IZX"}, 8 }},
      {0x64, { {&Executioner::opDOP, "DOP"}, {&Executioner::addrZP0, "ZP0"}, 3 }},
#endif
      {0x65, { {&Executioner::opADC, "ADC"}, {&Executioner::addrZP0, "ZP0"}, 3 }},
      {0x66, { {&Executioner::opROR, "ROR"}, {&Executioner::addrZP0, "ZP0"}, 5 }},
#if defined(ILLEGAL)
      {0x67, { {&Executioner::opRLA, "RLA"}, {&Executioner::addrZP0, "ZP0"}, 5 }},
#endif
      {0x68, { {&Executioner::opPLA, "PLA"}, {&Executioner::addrIMP, "IMP"}, 4 }},
      {0x69, { {&Executioner::opADC, "ADC"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0x6A, { {&Executioner::opROR, "ROR"}, {&Executioner::addrACC, "ACC"}, 2 }},
#if defined(ILLEGAL)
      {0x6B, { {&Executioner::opARR, "ARR"}, {&Executioner::addrIMM, "IMM"}, 2 }},
#endif
      {0x6C, { {&Executioner::opJMP, "JMP"}, {&Executioner::addrIND, "IND"}, 5 }},
      {0x6D, { {&Executioner::opADC, "ADC"}, {&Executioner::addrABS, "ABS"}, 4 }},
      {0x6E, { {&Executioner::opROR, "ROR"}, {&Executioner::addrABS, "ABS"}, 6 }},
#if defined(ILLEGAL)
      {0x6F, { {&Executioner::opRLA, "RLA"}, {&Executioner::addrABS, "ABS"}, 6 }},
#endif
#pragma endregion Instructions 0x6

#pragma region Instructions 0x7
      {0x70, { {&Executioner::opBVS, "BVS"}, {&Executioner::addrREL, "REL"}, 2 }},
      {0x71, { {&Executioner::opADC, "ADC"}, {&Executioner::addrIZY, "IZY"}, 5 }},
#if defined(ILLEGAL)
      {0x72, { {&Executioner::opJAM, "JAM"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0x73, { {&Executioner::opRLA, "RLA"}, {&Executioner::addrIZY, "IZY"}, 8 }},
      {0x74, { {&Executioner::opDOP, "DOP"}, {&Executioner::addrZPX, "ZPX"}, 4 }},
#endif
      {0x75, { {&Executioner::opADC, "ADC"}, {&Executioner::addrZPX, "ZPX"}, 4 }},
      {0x76, { {&Executioner::opROR, "ROR"}, {&Executioner::addrZPX, "ZPX"}, 6 }},
#if defined(ILLEGAL)
      {0x77, { {&Executioner::opRLA, "RLA"}, {&Executioner::addrZPX, "ZPX"}, 6 }},
#endif
      {0x78, { {&Executioner::opSEI, "SEI"}, {&Executioner::addrIMP, "IMP"}, 2 }},
      {0x79, { {&Executioner::opADC, "ADC"}, {&Executioner::addrABY, "ABY"}, 4 }},
#if defined(ILLEGAL)
      {0x7A, { {&Executioner::opNOP, "NOP"}, {&Executioner::addrIMP, "IMP"}, 2 }},
      {0x7B, { {&Executioner::opRLA, "RLA"}, {&Executioner::addrABY, "ABY"}, 7 }},
      {0x7C, { {&Executioner::opTOP, "TOP"}, {&Executioner::addrABX, "ABX"}, 4 }},
#endif
      {0x7D, { {&Executioner::opADC, "ADC"}, {&Executioner::addrABX, "ABX"}, 4 }},
#if defined(EMULATE65C02)
      {0x7E, { {&Executioner::opROR, "ROR"}, {&Executioner::addrABX, "ABX"}, 6 }},
#else
      {0x7E, { {&Executioner::opROR, "ROR"}, {&Executioner::addrABX, "ABX"}, 7 }},
#endif
#if defined(ILLEGAL)
      {0x7F, { {&Executioner::opRLA, "RLA"}, {&Executioner::addrABX, "ABX"}, 7 }},
#endif
#pragma endregion Instructions 0x7

#pragma region Instructions 0x8
#if defined(ILLEGAL)
      {0x80, { {&Executioner::opDOP, "DOP"}, {&Executioner::addrIMM, "IMM"}, 2 }},
#endif
      {0x81, { {&Executioner::opSTA, "STA"}, {&Executioner::addrIZX, "IZX"}, 6 }},
#if defined(ILLEGAL)
      {0x82, { {&Executioner::opDOP, "DOP"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0x83, { {&Executioner::opSAX, "SAX"}, {&Executioner::addrIZX, "IZX"}, 6 }},
#endif
      {0x84, { {&Executioner::opSTY, "STY"}, {&Executioner::addrZP0, "ZP0"}, 3 }},
      {0x85, { {&Executioner::opSTA, "STA"}, {&Executioner::addrZP0, "ZP0"}, 3 }},
      {0x86, { {&Executioner::opSTX, "STX"}, {&Executioner::addrZP0, "ZP0"}, 3 }},
#if defined(ILLEGAL)
      {0x87, { {&Executioner::opSAX, "SAX"}, {&Executioner::addrZP0, "ZP0"}, 3 }},
#endif
      {0x88, { {&Executioner::opDEY, "DEY"}, {&Executioner::addrIMP, "IMP"}, 2 }},
#if defined(ILLEGAL)

      {0x89, { {&Executioner::opDOP, "DOP"}, {&Executioner::addrIMM, "IMM"}, 2 }},
#endif
      {0x8A, { {&Executioner::opTXA, "TXA"}, {&Executioner::addrIMP, "IMP"}, 2 }},
#if defined(ILLEGAL)
      {0x8B, { {&Executioner::opANE, "ANE"}, {&Executioner::addrIMM, "IMM"}, 2 }},
#endif
      {0x8C, { {&Executioner::opSTY, "STY"}, {&Executioner::addrABS, "ABS"}, 4 }},
      {0x8D, { {&Executioner::opSTA, "STA"}, {&Executioner::addrABS, "ABS"}, 4 }},
      {0x8E, { {&Executioner::opSTX, "STX"}, {&Executioner::addrABS, "ABS"}, 4 }},
#if defined(ILLEGAL)
      {0x8F, { {&Executioner::opSAX, "SAX"}, {&Executioner::addrABS, "ABS"}, 4 }},
#endif
#pragma endregion Instructions 0x8

#pragma region Instructions 0x9
      {0x90, { {&Executioner::opBCC, "BCC"}, {&Executioner::addrREL, "REL"}, 2 }},
      {0x91, { {&Executioner::opSTA, "STA"}, {&Executioner::addrIZY, "IZY"}, 6 }},
#if defined(ILLEGAL)
      {0x92, { {&Executioner::opJAM, "JAM"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0x93, { {&Executioner::opSHA, "SHA"}, {&Executioner::addrIZY, "IZY"}, 6 }},
#endif
      {0x94, { {&Executioner::opSTY, "STY"}, {&Executioner::addrZPX, "ZPX"}, 4 }},
      {0x95, { {&Executioner::opSTA, "STA"}, {&Executioner::addrZPX, "ZPX"}, 4 }},
      {0x96, { {&Executioner::opSTX, "STX"}, {&Executioner::addrZPY, "ZPY"}, 4 }},
#if defined(ILLEGAL)
      {0x97, { {&Executioner::opSAX, "SAX"}, {&Executioner::addrZPY, "ZPY"}, 4 }},
#endif
      {0x98, { {&Executioner::opTYA, "TYA"}, {&Executioner::addrIMP, "IMP"}, 2 }},
      {0x99, { {&Executioner::opSTA, "STA"}, {&Executioner::addrABY, "ABY"}, 5 }},
      {0x9A, { {&Executioner::opTXS, "TXS"}, {&Executioner::addrIMP, "IMP"}, 2 }},
#if defined(ILLEGAL)
      {0x9B, { {&Executioner::opTAS, "TAS"}, {&Executioner::addrABY, "ABY"}, 5 }},
      {0x9C, { {&Executioner::opSHY, "SHY"}, {&Executioner::addrABX, "ABX"}, 5 }},
#endif
      {0x9D, { {&Executioner::opSTA, "STA"}, {&Executioner::addrABX, "ABX"}, 5 }},
#if defined(ILLEGAL)
      {0x9E, { {&Executioner::opSHX, "SHX"}, {&Executioner::addrABY, "ABY"}, 5 }},
      {0x9F, { {&Executioner::opSHA, "SHA"}, {&Executioner::addrABY, "ABY"}, 5 }},
#endif
#pragma endregion Instructions 0x9

#pragma region Instructions 0xA
      {0xA0, { {&Executioner::opLDY, "LDY"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0xA1, { {&Executioner::opLDA, "LDA"}, {&Executioner::addrIZX, "IZX"}, 6 }},
      {0xA2, { {&Executioner::opLDX, "LDX"}, {&Executioner::addrIMM, "IMM"}, 2 }},
#if defined(ILLEGAL)
      {0xA3, { {&Executioner::opLAX, "LAX"}, {&Executioner::addrIZX, "IZX"}, 6 }},
#endif
      {0xA4, { {&Executioner::opLDY, "LDY"}, {&Executioner::addrZP0, "ZP0"}, 3 }},
      {0xA5, { {&Executioner::opLDA, "LDA"}, {&Executioner::addrZP0, "ZP0"}, 3 }},
      {0xA6, { {&Executioner::opLDX, "LDX"}, {&Executioner::addrZP0, "ZP0"}, 3 }},
#if defined(ILLEGAL)
      {0xA7, { {&Executioner::opLAX, "LAX"}, {&Executioner::addrZP0, "ZP0"}, 3 }},
#endif
      {0xA8, { {&Executioner::opTAY, "TAY"}, {&Executioner::addrIMP, "IMP"}, 2 }},
      {0xA9, { {&Executioner::opLDA, "LDA"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0xAA, { {&Executioner::opTAX, "TAX"}, {&Executioner::addrIMP, "IMP"}, 2 }},
#if defined(ILLEGAL)
      {0xAB, { {&Executioner::opLXA, "LXA"}, {&Executioner::addrIMM, "IMM"}, 2 }},
#endif
      {0xAC, { {&Executioner::opLDY, "LDY"}, {&Executioner::addrABS, "ABS"}, 4 }},
      {0xAD, { {&Executioner::opLDA, "LDA"}, {&Executioner::addrABS, "ABS"}, 4 }},
      {0xAE, { {&Executioner::opLDX, "LDX"}, {&Executioner::addrABS, "ABS"}, 4 }},
#if defined(ILLEGAL)
      {0xAF, { {&Executioner::opLAX, "LAX"}, {&Executioner::addrABS, "ABS"}, 4 }},
#endif
#pragma endregion Instructions 0xA

#pragma region Instructions 0xB
      {0xB0, { {&Executioner::opBCS, "BCS"}, {&Executioner::addrREL, "REL"}, 2 }},
      {0xB1, { {&Executioner::opLDA, "LDA"}, {&Executioner::addrIZY, "IZY"}, 5 }},
#if defined(ILLEGAL)
      {0xB2, { {&Executioner::opJAM, "JAM"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0xB3, { {&Executioner::opLAX, "LAX"}, {&Executioner::addrIZY, "IZY"}, 5 }},
#endif
      {0xB4, { {&Executioner::opLDY, "LDY"}, {&Executioner::addrZPX, "ZPX"}, 4 }},
      {0xB5, { {&Executioner::opLDA, "LDA"}, {&Executioner::addrZPX, "ZPX"}, 4 }},
      {0xB6, { {&Executioner::opLDX, "LDX"}, {&Executioner::addrZPY, "ZPY"}, 4 }},
#if defined(ILLEGAL)
      {0xB7, { {&Executioner::opLAX, "LAX"}, {&Executioner::addrZPX, "ZPX"}, 4 }},
#endif
      {0xB8, { {&Executioner::opCLV, "CLV"}, {&Executioner::addrIMP, "IMP"}, 2 }},
      {0xB9, { {&Executioner::opLDA, "LDA"}, {&Executioner::addrABY, "ABY"}, 4 }},
      {0xBA, { {&Executioner::opTSX, "TSX"}, {&Executioner::addrIMP, "IMP"}, 2 }},
#if defined(ILLEGAL)
      {0xBB, { {&Executioner::opLAS, "LAS"}, {&Executioner::addrABY, "ABY"}, 4 }},
#endif
      {0xBC, { {&Executioner::opLDY, "LDY"}, {&Executioner::addrABX, "ABX"}, 4 }},
      {0xBD, { {&Executioner::opLDA, "LDA"}, {&Executioner::addrABX, "ABX"}, 4 }},
      {0xBE, { {&Executioner::opLDX, "LDX"}, {&Executioner::addrABY, "ABY"}, 4 }},
#if defined(ILLEGAL)
      {0xBF, { {&Executioner::opLAX, "LAX"}, {&Executioner::addrABY, "ABY"}, 4 }},
#endif
#pragma endregion Instructions 0xB

#pragma region Instructions 0xC
      {0xC0, { {&Executioner::opCPY, "CPY"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0xC1, { {&Executioner::opCMP, "CMP"}, {&Executioner::addrIZX, "IZX"}, 6 }},
#if defined(ILLEGAL)
      {0xC2, { {&Executioner::opDOP, "DOP"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0xC3, { {&Executioner::opDCP, "DCP"}, {&Executioner::addrIZX, "IZX"}, 8 }},
#endif
      {0xC4, { {&Executioner::opCPY, "CPY"}, {&Executioner::addrZP0, "ZP0"}, 3 }},
      {0xC5, { {&Executioner::opCMP, "CMP"}, {&Executioner::addrZP0, "ZP0"}, 3 }},
      {0xC6, { {&Executioner::opDEC, "DEC"}, {&Executioner::addrZP0, "ZP0"}, 5 }},
#if defined(ILLEGAL)
      {0xC7, { {&Executioner::opDCP, "DCP"}, {&Executioner::addrZP0, "ZP0"}, 5 }},
#endif
      {0xC8, { {&Executioner::opINY, "INY"}, {&Executioner::addrIMP, "IMP"}, 2 }},
      {0xC9, { {&Executioner::opCMP, "CMP"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0xCA, { {&Executioner::opDEX, "DEX"}, {&Executioner::addrIMP, "IMP"}, 2 }},
#if defined(EMULATE65C02)
      {0xCB, { {&Executioner::opWAI, "WAI"}, {&Executioner::addrIMM, "IMP"}, 3 }},
#elif defined(ILLEGAL)
      {0xCB, { {&Executioner::opSBX, "SBX"}, {&Executioner::addrIMM, "IMM"}, 2 }},
#endif
      {0xCC, { {&Executioner::opCPY, "CPY"}, {&Executioner::addrABS, "ABS"}, 4 }},
      {0xCD, { {&Executioner::opCMP, "CMP"}, {&Executioner::addrABS, "ABS"}, 4 }},
      {0xCE, { {&Executioner::opDEC, "DEC"}, {&Executioner::addrABS, "ABS"}, 6 }},
#if defined(ILLEGAL)
      {0xCF, { {&Executioner::opDCP, "DCP"}, {&Executioner::addrABS, "ABS"}, 6 }},
#endif
#pragma endregion Instructions 0xC

#pragma region Instructions 0xD
      {0xD0, { {&Executioner::opBNE, "BNE"}, {&Executioner::addrREL, "REL"}, 2 }},
      {0xD1, { {&Executioner::opCMP, "CMP"}, {&Executioner::addrIZY, "IZY"}, 5 }},
#if defined(ILLEGAL)
      {0xD2, { {&Executioner::opJAM, "JAM"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0xD3, { {&Executioner::opDCP, "DCP"}, {&Executioner::addrIZY, "IZY"}, 8 }},
      {0xD4, { {&Executioner::opDOP, "DOP"}, {&Executioner::addrZPX, "ZPX"}, 4 }},
#endif
      {0xD5, { {&Executioner::opCMP, "CMP"}, {&Executioner::addrZPX, "ZPX"}, 4 }},
      {0xD6, { {&Executioner::opDEC, "DEC"}, {&Executioner::addrZPX, "ZPX"}, 6 }},
#if defined(ILLEGAL)
      {0xD7, { {&Executioner::opDCP, "DCP"}, {&Executioner::addrZPX, "ZPX"}, 6 }},
#endif
      {0xD8, { {&Executioner::opCLD, "CLD"}, {&Executioner::addrIMP, "IMP"}, 2 }},
      {0xD9, { {&Executioner::opCMP, "CMP"}, {&Executioner::addrABY, "ABY"}, 4 }},
#if defined(ILLEGAL)
      {0xDA, { {&Executioner::opNOP, "NOP"}, {&Executioner::addrIMP, "IMP"}, 2 }},
#endif
#if defined(EMULATE65C02)
      {0xDB, { {&Executioner::opSTP, "STP"}, {&Executioner::addrIMP, "IMP"}, 7 }},
#elif defined(ILLEGAL)
      {0xDB, { {&Executioner::opDCP, "DCP"}, {&Executioner::addrABY, "ABY"}, 7 }},
#endif
#if defined(ILLEGAL)
      {0xDC, { {&Executioner::opTOP, "TOP"}, {&Executioner::addrABX, "ABX"}, 4 }},
#endif
      {0xDD, { {&Executioner::opCMP, "CMP"}, {&Executioner::addrABX, "ABX"}, 4 }},
      {0xDE, { {&Executioner::opDEC, "DEC"}, {&Executioner::addrABX, "ABX"}, 7 }},
#if defined(ILLEGAL)
      {0xDF, { {&Executioner::opDCP, "DCP"}, {&Executioner::addrABX, "ABX"}, 7 }},
#endif
#pragma endregion Instructions 0xD

#pragma region Instructions 0xE
      {0xE0, { {&Executioner::opCPX, "CPX"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0xE1, { {&Executioner::opSBC, "SBC"}, {&Executioner::addrIZX, "IZX"}, 6 }},
#if defined(ILLEGAL)
      {0xE2, { {&Executioner::opDOP, "DOP"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0xE3, { {&Executioner::opISC, "ISC"}, {&Executioner::addrIZX, "IZX"}, 8 }},
#endif
      {0xE4, { {&Executioner::opCPX, "CPX"}, {&Executioner::addrZP0, "ZP0"}, 3 }},
      {0xE5, { {&Executioner::opSBC, "SBC"}, {&Executioner::addrZP0, "ZP0"}, 3 }},
      {0xE6, { {&Executioner::opINC, "INC"}, {&Executioner::addrZP0, "ZP0"}, 5 }},
#if defined(ILLEGAL)
      {0xE7, { {&Executioner::opISC, "ISC"}, {&Executioner::addrZP0, "ZP0"}, 5 }},
#endif
      {0xE8, { {&Executioner::opINX, "INX"}, {&Executioner::addrIMP, "IMP"}, 2 }},
      {0xE9, { {&Executioner::opSBC, "SBC"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0xEA, { {&Executioner::opNOP, "NOP"}, {&Executioner::addrIMP, "IMP"}, 2 }},
#if defined(ILLEGAL)
      {0xEB, { {&Executioner::opUSBC, "USBC"}, {&Executioner::addrIMM, "IMM"}, 2 }},
#endif
      {0xEC, { {&Executioner::opCPX, "CPX"}, {&Executioner::addrABS, "ABS"}, 4 }},
      {0xED, { {&Executioner::opSBC, "SBC"}, {&Executioner::addrABS, "ABS"}, 4 }},
      {0xEE, { {&Executioner::opINC, "INC"}, {&Executioner::addrABS, "ABS"}, 6 }},
#if defined(ILLEGAL)
      {0xEF, { {&Executioner::opISC, "ISC"}, {&Executioner::addrABS, "ABS"}, 6 }},
#endif
#pragma endregion Instructions 0xE

#pragma region Instructions 0xF
      {0xF0, { {&Executioner::opBEQ, "BEQ"}, {&Executioner::addrREL, "REL"}, 2 }},
      {0xF1, { {&Executioner::opSBC, "SBC"}, {&Executioner::addrIZY, "IZY"}, 5 }},
#if defined(ILLEGAL)
      {0xF2, { {&Executioner::opJAM, "JAM"}, {&Executioner::addrIMM, "IMM"}, 2 }},
      {0xF3, { {&Executioner::opISC, "ISC"}, {&Executioner::addrIZY, "IZY"}, 8 }},
      {0xF4, { {&Executioner::opDOP, "DOP"}, {&Executioner::addrZPX, "ZPX"}, 4 }},
#endif
      {0xF5, { {&Executioner::opSBC, "SBC"}, {&Executioner::addrZPX, "ZPX"}, 4 }},
      {0xF6, { {&Executioner::opINC, "INC"}, {&Executioner::addrZPX, "ZPX"}, 6 }},
#if defined(ILLEGAL)
      {0xF7, { {&Executioner::opISC, "ISC"}, {&Executioner::addrZPX, "ZPX"}, 6 }},
#endif
      {0xF8, { {&Executioner::opSED, "SED"}, {&Executioner::addrIMP, "IMP"}, 2 }},
      {0xF9, { {&Executioner::opSBC, "SBC"}, {&Executioner::addrABY, "ABY"}, 4 }},
#if defined(ILLEGAL)
      {0xFA, { {&Executioner::opNOP, "NOP"}, {&Executioner::addrIMP, "IMP"}, 2 }},
      {0xFB, { {&Executioner::opISC, "ISC"}, {&Executioner::addrABY, "ABY"}, 7 }},
      {0xFC, { {&Executioner::opTOP, "TOP"}, {&Executioner::addrABX, "ABX"}, 4 }},
#endif
      {0xFD, { {&Executioner::opSBC, "SBC"}, {&Executioner::addrABX, "ABX"}, 4 }},
      {0xFE, { {&Executioner::opINC, "INC"}, {&Executioner::addrABX, "ABX"}, 7 }},
#if defined(ILLEGAL)
      {0xFF, { {&Executioner::opISC, "ISC"}, {&Executioner::addrABX, "ABX"}, 7 }},
#endif
#pragma endregion Instructions 0xF
    };
#pragma endregion Instructions
  };
};
