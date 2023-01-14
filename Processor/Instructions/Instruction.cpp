#include "Instruction.hpp"
#include "../Executioner.hpp"
#include <map>

namespace CPU
{
  namespace Instructions
  {
    Instruction::Instruction()
    {
      using I = CPU::Executioner;
      InstructionSet instSet;

      lookup = {
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

    };

    uint8_t Instruction::exists(uint8_t opcode)
    {
      return !(lookup.find(opcode) == lookup.end());
    }

    Executioner::tMode Instruction::get(uint8_t opcode)
    {
      return lookup[opcode];
    }

  };
};
