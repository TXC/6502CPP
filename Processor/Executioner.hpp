#pragma once

#include <string>
#include <vector>
#include <map>

#ifdef SPDLOG_FMT_EXTERNAL
#include <fmt/format.h>
#include <fmt/ostream.h>
#else
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ostr.h>
#endif

#ifdef LOGMODE
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#endif

namespace CPU
{
  class Processor;
  class Logger;
  class Executioner
  {
  public:
    Executioner();
    ~Executioner();

  public:
    void ConnectCpu(Processor* n) { cpu = n; }

    void breakOperation(bool isBreak, uint16_t vector);
    void branchOperation(bool performBranch);

    void reset();

  public:
    typedef uint8_t(Executioner::* ExecutionType)(void);

    // This structure are used to compile and store
    // the opcode translation tables.
    struct tMode
    {
	    // Function: A function pointer to the implementation of the opcode
      ExecutionType op = nullptr;
      // Mnemonic: A textual representation of the instruction (used for disassembly)
      std::string name;

      friend std::ostream &operator<<(std::ostream &os, const tMode& obj)
      {
        fmt::format_to(std::ostream_iterator<char>(os), "{}", obj.name);
        return os;
      }
    };

    // This structure and the following map are used to compile and store
    // the opcode translation tables.
    struct OperationType
    {
      // Opcode Function : The opcode used by the instruction
      tMode   operate;
      // Opcode Address Mode : The addressing mechanism used by the instruction
      tMode   addrmode;
      // Cycle Count : An integer that represents the base number of clock cycles the
      //               CPU requires to perform the instruction
      uint8_t cycles = 0;

      friend std::ostream &operator<<(std::ostream &os, const OperationType& obj)
      {
        fmt::format_to(
          std::ostream_iterator<char>(os),
          "{}:{} [{:d}]",
          obj.operate.name, obj.addrmode.name, obj.cycles
        );
        return os;
      }
    };

    std::map<uint8_t, OperationType> lookup = {};


  private:
    Processor* cpu = nullptr;

    // Is the instruction byte
    uint8_t* opcode = nullptr;
    // Counts how many cycles the instruction has remaining
    uint8_t* cycle_count = nullptr;
    // A global accumulation of the number of clocks
    uint32_t* clock_count = nullptr;

    // Assistive variables to facilitate emulation

    // 8-bit - Represents the working input value to the ALU
    uint8_t  fetched = 0x00;
    // 8-bit - A convenience variable used everywhere
    // @deprecated
    uint16_t temp = 0x0000;
    // 16-bit - All used memory addresses end up in here
    uint16_t addr_abs = 0x0000;
    // 16bit - Represents absolute address following a branch
    uint16_t addr_rel = 0x0000;

    // The read location of data can come from two sources, a memory address, or
    // its immediately available as part of the instruction. This function decides
    // depending on address mode of instruction byte
    uint8_t fetch();

    // Populates all instructions for the 6502 CPU into the lookup-map
    void loadInstructions();

    // Internal use methods, uses the currently loaded opcode
  private:
    uint8_t execute();
    std::string getInstructionName();
    std::string getAddressModeName();
    std::string getOperation();

    // External use methods, uses opcode via arguments
  public:
    uint8_t execute(uint8_t opcode);
    // Helper method, returns mnemonic for instruction
    std::string getInstructionName(uint8_t opcode);
    // Helper method, returns mnemonic for address mode
    std::string getAddressModeName(uint8_t opcode);
    // Helper method, returns instruction mnemonics (INST:ADDR [opcode])
    std::string getOperation(uint8_t opcode);


  public:
    // Addressing Modes =============================================
    // The 6502 has a variety of addressing modes to access data in 
    // memory, some of which are direct and some are indirect (like
    // pointers in C++). Each opcode contains information about which
    // addressing mode should be employed to facilitate the 
    // instruction, in regards to where it reads/writes the data it
    // uses. The address mode changes the number of bytes that
    // makes up the full instruction, so we implement addressing
    // before executing the instruction, to make sure the program
    // counter is at the correct location, the instruction is
    // primed with the addresses it needs, and the number of clock
    // cycles the instruction requires is calculated. These functions
    // may adjust the number of cycles required depending upon where
    // and how the memory is accessed, so they return the required
    // adjustment.

    uint8_t ACC();
    uint8_t IMP();
    uint8_t IMM();
    uint8_t REL();
    uint8_t ZP0();
    uint8_t ZPX();
    uint8_t ZPY();
    uint8_t ABS();
    uint8_t ABX();
    uint8_t ABY();
    uint8_t IND();
    uint8_t IZX();
    uint8_t IZY();

    // Opcodes ======================================================
    // There are 56 "legitimate" opcodes provided by the 6502 CPU. I
    // have not modelled "unofficial" opcodes. As each opcode is 
    // defined by 1 byte, there are potentially 256 possible codes.
    // Codes are not used in a "switch case" style on a processor,
    // instead they are repsonisble for switching individual parts of
    // CPU circuits on and off. The opcodes listed here are official, 
    // meaning that the functionality of the chip when provided with
    // these codes is as the developers intended it to be. Unofficial
    // codes will of course also influence the CPU circuitry in 
    // interesting ways, and can be exploited to gain additional
    // functionality!
    //
    // These functions return 0 normally, but some are capable of
    // requiring more clock cycles when executed under certain
    // conditions combined with certain addressing modes. If that is 
    // the case, they return 1.
    //
    // I have included detailed explanations of each function in 
    // the class implementation file. Note they are listed in
    // alphabetical order here for ease of finding.

    uint8_t ADC();
    uint8_t AND();
    uint8_t ASL();
    uint8_t BCC();
    uint8_t BCS();
    uint8_t BEQ();
    uint8_t BIT();
    uint8_t BMI();
    uint8_t BNE();
    uint8_t BPL();
    uint8_t BRK();
    uint8_t BVC();
    uint8_t BVS();
    uint8_t CLC();
    uint8_t CLD();
    uint8_t CLI();
    uint8_t CLV();
    uint8_t CMP();
    uint8_t CPX();
    uint8_t CPY();
    uint8_t DEC();
    uint8_t DEX();
    uint8_t DEY();
    uint8_t EOR();
    uint8_t INC();
    uint8_t INX();
    uint8_t INY();
    uint8_t JMP();
    uint8_t JSR();
    uint8_t LDA();
    uint8_t LDX();
    uint8_t LDY();
    uint8_t LSR();
    uint8_t NOP();
    uint8_t ORA();
    uint8_t PHA();
    uint8_t PHP();
    uint8_t PLA();
    uint8_t PLP();
    uint8_t ROL();
    uint8_t ROR();
    uint8_t RTI();
    uint8_t RTS();
    uint8_t SBC();
    uint8_t SEC();
    uint8_t SED();
    uint8_t SEI();
    uint8_t STA();
    uint8_t STX();
    uint8_t STY();
    uint8_t TAX();
    uint8_t TAY();
    uint8_t TSX();
    uint8_t TXA();
    uint8_t TXS();
    uint8_t TYA();

#ifdef ILLEGAL
    // Illegal opcodes
    uint8_t ALR();
    uint8_t ANC();
    uint8_t ANC2();
    uint8_t ANE();
    uint8_t ARR();
    uint8_t DCP();
    uint8_t ISC();
    uint8_t LAS();
    uint8_t LAX();
    uint8_t LXA();
    uint8_t RLA();
    uint8_t RRA();
    uint8_t SAX();
    uint8_t SBX();
    uint8_t SHA();
    uint8_t SHX();
    uint8_t SHY();
    uint8_t SLO();
    uint8_t SRE();
    uint8_t TAS();
    uint8_t USBC();
    uint8_t DOP();
    uint8_t TOP();
    uint8_t JAM();
#endif

#ifndef ILLEGAL
    // I capture all "unofficial" opcodes with this function. It is
    // functionally identical to a NOP
    uint8_t XXX();
#endif

#ifdef ILLEGAL
  private:
    // Magic value that depends on manufacturer and/or batch 
    // https://www.nesdev.org/wiki/Visual6502wiki/6502_Opcode_8B_(XAA,_ANE)
    // Can be 0xFF, 0xFE, 0xEE & 0x00 or something else
    uint8_t  magic = 0xFF;
#endif
  };
}
