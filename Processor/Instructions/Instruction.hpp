#pragma once

#include <map>
#include "../Singleton.hpp"
#include "../Executioner.hpp"

namespace CPU
{
  class Executioner;
  namespace Instructions
  {
    class Instruction final : public Singleton<Instruction>
    {
    private:
      std::map<uint8_t, Executioner::tMode> lookup = {};

    public:
      struct InstructionSet
      {
        static const uint8_t ADC = 0;
        static const uint8_t AND = 1;
        static const uint8_t ASL = 2;
        static const uint8_t BCC = 3;
        static const uint8_t BCS = 4;
        static const uint8_t BEQ = 5;
        static const uint8_t BIT = 6;
        static const uint8_t BMI = 7;
        static const uint8_t BNE = 8;
        static const uint8_t BPL = 9;
        static const uint8_t BRK = 10;
        static const uint8_t BVC = 11;
        static const uint8_t BVS = 12;
        static const uint8_t CLC = 13;
        static const uint8_t CLD = 14;
        static const uint8_t CLI = 15;
        static const uint8_t CLV = 16;
        static const uint8_t CMP = 17;
        static const uint8_t CPX = 18;
        static const uint8_t CPY = 19;
        static const uint8_t DEC = 20;
        static const uint8_t DEX = 21;
        static const uint8_t DEY = 22;
        static const uint8_t EOR = 23;
        static const uint8_t INC = 24;
        static const uint8_t INX = 25;
        static const uint8_t INY = 26;
        static const uint8_t JMP = 27;
        static const uint8_t JSR = 28;
        static const uint8_t LDA = 29;
        static const uint8_t LDX = 30;
        static const uint8_t LDY = 31;
        static const uint8_t LSR = 32;
        static const uint8_t NOP = 33;
        static const uint8_t ORA = 34;
        static const uint8_t PHA = 35;
        static const uint8_t PHP = 36;
        static const uint8_t PLA = 37;
        static const uint8_t PLP = 38;
        static const uint8_t ROL = 39;
        static const uint8_t ROR = 40;
        static const uint8_t RTI = 41;
        static const uint8_t RTS = 42;
        static const uint8_t SBC = 43;
        static const uint8_t SEC = 44;
        static const uint8_t SED = 45;
        static const uint8_t SEI = 46;
        static const uint8_t STA = 47;
        static const uint8_t STX = 48;
        static const uint8_t STY = 49;
        static const uint8_t TAX = 50;
        static const uint8_t TAY = 51;
        static const uint8_t TSX = 52;
        static const uint8_t TXA = 53;
        static const uint8_t TXS = 54;
        static const uint8_t TYA = 55;
#ifdef ILLEGAL
        static const uint8_t ANC = 57;
        static const uint8_t ALR = 56;
        static const uint8_t ANC2 = 58;
        static const uint8_t ANE = 59;
        static const uint8_t ARR = 60;
        static const uint8_t DCP = 61;
        static const uint8_t ISC = 62;
        static const uint8_t LAS = 63;
        static const uint8_t LAX = 64;
        static const uint8_t LXA = 65;
        static const uint8_t RLA = 66;
        static const uint8_t RRA = 67;
        static const uint8_t SAX = 68;
        static const uint8_t SBX = 69;
        static const uint8_t SHA = 70;
        static const uint8_t SHX = 71;
        static const uint8_t SHY = 72;
        static const uint8_t SLO = 73;
        static const uint8_t SRE = 74;
        static const uint8_t TAS = 75;
        static const uint8_t USBC = 76;
        static const uint8_t DOP = 77;
        static const uint8_t TOP = 78;
        static const uint8_t JAM = 79;
#else
        static const uint8_t XXX = 255;
#endif
      };

    protected:
      Instruction();

    public:
      uint8_t exists(uint8_t opcode);
      Executioner::tMode get(uint8_t opcode);
    };
  };
};
