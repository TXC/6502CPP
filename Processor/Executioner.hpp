#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>

#include <spdlog/spdlog.h>
#if defined SPDLOG_FMT_EXTERNAL
#include <fmt/format.h>
#include <fmt/ostream.h>
#else
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ostr.h>
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
    typedef void(Executioner::* ExecutionType)(void);

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
    //static std::map<uint8_t, OperationType> lookup;


  private:
    Processor* cpu = nullptr;

    // Is the instruction byte
    uint8_t* opcode = nullptr;
    // Counts how many cycles the instruction has remaining
    uint8_t* cycle_count = nullptr;
    // Operation/Tick cycle
    uint32_t* operation_cycle = nullptr;

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
    void execute();
    std::string getInstructionName();
    std::string getAddressModeName();
    std::string getOperation();

    // External use methods, uses opcode via arguments
  public:
    void execute(uint8_t opcode);
    // Helper method, returns mnemonic for instruction
    std::string getInstructionName(uint8_t opcode);
    // Helper method, returns mnemonic for address mode
    std::string getAddressModeName(uint8_t opcode);
    // Helper method, returns instruction mnemonics (INST:ADDR [opcode])
    std::string getOperation(uint8_t opcode);

    void printInstructions();

  private:
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

    void addrACC();
    void addrIMP();
    void addrIMM();
    void addrREL();
    void addrZP0();
    void addrZPX();
    void addrZPY();
    void addrABS();
    void addrABX();
    void addrABY();
    void addrIND();
    void addrIZX();
    void addrIZY();

  private:
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

    void opADC();
    void opAND();
    void opASL();
    void opBCC();
    void opBCS();
    void opBEQ();
    void opBIT();
    void opBMI();
    void opBNE();
    void opBPL();
    void opBRK();
    void opBVC();
    void opBVS();
    void opCLC();
    void opCLD();
    void opCLI();
    void opCLV();
    void opCMP();
    void opCPX();
    void opCPY();
    void opDEC();
    void opDEX();
    void opDEY();
    void opEOR();
    void opINC();
    void opINX();
    void opINY();
    void opJMP();
    void opJSR();
    void opLDA();
    void opLDX();
    void opLDY();
    void opLSR();
    void opNOP();
    void opORA();
    void opPHA();
    void opPHP();
    void opPLA();
    void opPLP();
    void opROL();
    void opROR();
    void opRTI();
    void opRTS();
    void opSBC();
    void opSEC();
    void opSED();
    void opSEI();
    void opSTA();
    void opSTX();
    void opSTY();
    void opTAX();
    void opTAY();
    void opTSX();
    void opTXA();
    void opTXS();
    void opTYA();

    // I capture all "missing/illegal" opcodes with this function.
    void opXXX();

#if defined EMULATE65C02
    void opSTP();
    void opWAI();
#endif

#if defined ILLEGAL
    // Illegal opcodes
    void opALR();
    void opANC();
    void opANC2();
    void opANE();
    void opARR();
    void opDCP();
    void opISC();
    void opLAS();
    void opLAX();
    void opLXA();
    void opRLA();
    void opRRA();
    void opSAX();
    void opSBX();
    void opSHA();
    void opSHX();
    void opSHY();
    void opSLO();
    void opSRE();
    void opTAS();
    void opUSBC();
    void opDOP();
    void opTOP();
    void opJAM();

  private:
    // Magic value that depends on manufacturer and/or batch 
    // https://www.nesdev.org/wiki/Visual6502wiki/6502_Opcode_8B_(XAA,_ANE)
    // Can be 0xFF, 0xFE, 0xEE & 0x00 or something else
    uint8_t magic_ANE = 0xEF;
    uint8_t magic_LXA = 0xEE;
#endif

  };
}
