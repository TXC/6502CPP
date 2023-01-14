#pragma once

#include "Types.hpp"
//#include "Operation.hpp"

#include <string>
#include <vector>
#include <map>

#ifdef LOGMODE
#include <stdio.h>
#endif

namespace CPU
{
  class Processor;
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
    struct tMode
    {
      ExecutionType op = nullptr;
      std::string   name;
    };
  
  private:
    // This structure and the following map are used to compile and store
    // the opcode translation tables. The 6502 can effectively have 256
    // different instructions.
    // Each table entry holds:
    //  Opcode Function: A function pointer to the implementation of the opcode
    //  Opcode Address Mode : A function pointer to the implementation of the 
    //                        addressing mechanism used by the instruction
    //  Cycle Count : An integer that represents the base number of clock cycles the
    //                CPU requires to perform the instruction
    struct OperationType
    {
      uint8_t operate = 0x00;
      uint8_t addrmode = 0x00;
      uint8_t cycles = 0;
    };

    std::map<uint8_t, tMode> addressModes = {};
    std::map<uint8_t, tMode> operationCodes = {};
    std::map<uint8_t, OperationType> lookup = {};

  public:
    struct AddressingModes
    {
      static const uint8_t Accumulator = 0;
      static const uint8_t Implied = 1;
      static const uint8_t Immediate = 2;
      static const uint8_t Relative = 3;
      static const uint8_t Absolute = 4;
      static const uint8_t AbsoluteX = 5;
      static const uint8_t AbsoluteY = 6;
      static const uint8_t ZeroPage = 7;
      static const uint8_t ZeroPageX = 8;
      static const uint8_t ZeroPageY = 9;
      static const uint8_t Indirect = 10;
      static const uint8_t IndirectX = 11;
      static const uint8_t IndirectY = 12;
    };

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

  private:
    Processor* cpu = nullptr;

    uint8_t* opcode = nullptr;  // Is the instruction byte
    uint8_t* cycle_count = nullptr;  // Counts how many cycles the instruction has remaining
    uint32_t* clock_count = nullptr;  // A global accumulation of the number of clocks

    // Assistive variables to facilitate emulation
    uint8_t  fetched = 0x00;     // Represents the working input value to the ALU
    uint16_t temp = 0x0000;   // A convenience variable used everywhere
    uint16_t addr_abs = 0x0000;   // All used memory addresses end up in here
    uint16_t addr_rel = 0x00;     // Represents absolute address following a branch

    // The read location of data can come from two sources, a memory address, or
    // its immediately available as part of the instruction. This function decides
    // depending on address mode of instruction byte
    uint8_t fetch();

    void loadInstructions();

    uint8_t execute();
    uint8_t getAddressMode();
    const char* getInstructionName();
    const char* getAddressModeName();

  public:
    uint8_t execute(uint8_t opcode);
    uint8_t getAddressMode(uint8_t opcode);
    const char* getInstructionName(uint8_t opcode);
    const char* getAddressModeName(uint8_t opcode);


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
    uint8_t IMP();  uint8_t IMM();  uint8_t REL();
    uint8_t ZP0();  uint8_t ZPX();  uint8_t ZPY();
    uint8_t ABS();  uint8_t ABX();  uint8_t ABY();
    uint8_t IND();  uint8_t IZX();  uint8_t IZY();

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

    uint8_t ADC();  uint8_t AND();  uint8_t ASL();  uint8_t BCC();  uint8_t BCS();  uint8_t BEQ();  uint8_t BIT();
    uint8_t BMI();  uint8_t BNE();  uint8_t BPL();  uint8_t BRK();  uint8_t BVC();  uint8_t BVS();  uint8_t CLC();
    uint8_t CLD();  uint8_t CLI();  uint8_t CLV();  uint8_t CMP();  uint8_t CPX();  uint8_t CPY();  uint8_t DEC();
    uint8_t DEX();  uint8_t DEY();  uint8_t EOR();  uint8_t INC();  uint8_t INX();  uint8_t INY();  uint8_t JMP();
    uint8_t JSR();  uint8_t LDA();  uint8_t LDX();  uint8_t LDY();  uint8_t LSR();  uint8_t NOP();  uint8_t ORA();
    uint8_t PHA();  uint8_t PHP();  uint8_t PLA();  uint8_t PLP();  uint8_t ROL();  uint8_t ROR();  uint8_t RTI();
    uint8_t RTS();  uint8_t SBC();  uint8_t SEC();  uint8_t SED();  uint8_t SEI();  uint8_t STA();  uint8_t STX();
    uint8_t STY();  uint8_t TAX();  uint8_t TAY();  uint8_t TSX();  uint8_t TXA();  uint8_t TXS();  uint8_t TYA();

#ifdef ILLEGAL
    // Illegal opcodes
    uint8_t ALR();  uint8_t ANC();  uint8_t ANC2(); uint8_t ANE();  uint8_t ARR();  uint8_t DCP();  uint8_t ISC();
    uint8_t LAS();  uint8_t LAX();  uint8_t LXA();  uint8_t RLA();  uint8_t RRA();  uint8_t SAX();  uint8_t SBX();
    uint8_t SHA();  uint8_t SHX();  uint8_t SHY();  uint8_t SLO();  uint8_t SRE();  uint8_t TAS();  uint8_t USBC();
    uint8_t DOP();  uint8_t TOP();  uint8_t JAM();
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
