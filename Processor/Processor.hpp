#pragma once

#include "Executioner.hpp"

#include <vector>
#include <map>
#include <memory>
#include <string>
#include <spdlog/spdlog.h>
#ifdef SPDLOG_FMT_EXTERNAL
#include <fmt/format.h>
#include <fmt/ostream.h>
#else
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ostr.h>
#endif


namespace CPU
{
  // Forward declaration of generic communications bus class to
  // prevent circular inclusions
  class Bus;
  class Logger;
  class Executioner;

  // The 6502 Emulation Class. This is it!
  class Processor
  {
  public:
    Processor();
    ~Processor();

    // Linkages
  private:
    // Linkage to the communications bus
    Bus* bus = nullptr;
    bool jammed = 0x00;

  public:
    // Linkage to the instructions
    Executioner executioner;

    uint8_t  opcode = 0x00;     // Is the instruction byte
    uint8_t  extra_cycles = 0;  // Number of extra cycles that has been added
    uint8_t  cycle_count = 0;   // Counts how many cycles the instruction has remaining
    uint32_t clock_count = 0;   // A global accumulation of the number of clocks

    // When true, process interrupt
    bool _previousInterrupt = false;
    // If interrupt shall be triggered before next operation
    bool _interrupt = false;
    // Set to true when an NMI should occur
    bool TriggerNmi = false;
    // Set to true when an IRQ has occurred and is being processed by the CPU
    bool TriggerIRQ = false;

    // External event functions. In hardware these represent pins that are asserted
    // to produce a change in state.

    // Reset Interrupt - Forces CPU into known state
    void reset();
    // Interrupt Request - Executes an instruction at a specific location
    void irq();
    // Non-Maskable Interrupt Request - As above, but cannot be disabled
    void nmi();
    // Performs the next step on the processor
    void tick();
    // Set flag that CPU is in a "jammed" state, and requires reset.
    void setJammed();

    // Indicates the current instruction has completed by returning true. This is
    // a utility function to enable "step-by-step" execution, without manually 
    // clocking every cycle
    bool complete();

    // Load program
    void LoadProgram(uint16_t offset, std::string program);
    void LoadProgram(uint16_t offset, std::string program, uint16_t initialProgramCounter);
    void LoadProgram(uint16_t offset, uint8_t program[], size_t programSize);
    void LoadProgram(uint16_t offset, uint8_t program[], size_t programSize, uint16_t initialProgramCounter);

  // Bus Connectivity
  public:
    // Link this CPU to a communications bus
    void ConnectBus(Bus* n) { bus = n; }

    // Dump 16 bytes of RAM 0x???0 -> 0x???F to logfile
    void dumpRam(uint16_t offset);

    // Dump RAM from start offset to end offset to logfile
    void dumpRam(uint16_t offsetStart, uint16_t offsetStop);

  public:
    // The status register stores 8 flags. Ive enumerated these here for ease
    // of access. You can access the status register directly since its public.
    // The bits have different interpretations depending upon the context and 
    // instruction being executed.
    enum FLAGS6502
    {
      // Bit 0 - 0x01 - Carry Bit
      C = (1 << 0),
      // Bit 1 - 0x02 - Zero
      Z = (1 << 1),
      // Bit 2 - 0x04 - Disable Interrupts
      I = (1 << 2),
      // Bit 3 - 0x08 - Decimal Mode (unused in NES implementation)
      D = (1 << 3),
      // Bit 4 - 0x10 - Break
      B = (1 << 4),
      // Bit 5 - 0x20 - Unused
      U = (1 << 5),
      // Bit 6 - 0x40 - Overflow
      V = (1 << 6),
      // Bit 7 - 0x80 - Negative
      N = (1 << 7),
    };

    enum REGISTER6502
    {
      // 16-bit - Program Counter
      PC = 0,
      // 8-bit - Accumulator Register
      AC = 1,
      // 8-bit - X Register
      X = 2,
      // 8-bit - Y Register
      Y = 3,
      // 8-bit - Status Register
      SR = 4,
      // 8-bit - Stack Pointer (points to location on bus)
      SP = 5,
    };

    // CPU Core registers, exposed as public here for ease of access from external
    // examinors. This is all the 6502 has.
    struct REGISTER
    {
      // 8-bit - Accumulator Register
      uint8_t  AC = 0x00;
      // 8-bit - X Register
      uint8_t  X = 0x00;
      // 8-bit - Y Register
      uint8_t  Y = 0x00;
      // 8-bit - Stack Pointer (points to location on bus)
      uint8_t  SP = 0x00;
      // 16-bit - Program Counter
      uint16_t PC = 0x0000;
      // 8-bit - Status Register
      uint8_t  SR = 0x00;

      void reset()
      {
        // A RESET pushes SR & PC (3 bytes) to SP 0x100, therefore SP initializes to 0xFD
        *this = {
          .AC = 0x00,
          .X = 0x00,
          .Y = 0x00,
          .SP = 0xFD,
          .PC = 0x0000,
          .SR = 0x00 | FLAGS6502::U | FLAGS6502::B
        };
      }

      friend std::ostream &operator<<(std::ostream &os, const REGISTER& obj)
      {
        fmt::format_to(
          std::ostream_iterator<char>(os),
          "PC:{:04X} A:{:02X} X:{:02X} Y:{:02X} {}{}{}{}{}{}{}{} STKP:{:02X}",
          obj.PC, obj.AC, obj.X, obj.Y,
          (((obj.SR & FLAGS6502::N) > 0) ? "N" : "."),
          (((obj.SR & FLAGS6502::V) > 0) ? "V" : "."),
          (((obj.SR & FLAGS6502::U) > 0) ? "U" : "."),
          (((obj.SR & FLAGS6502::B) > 0) ? "B" : "."),
          (((obj.SR & FLAGS6502::D) > 0) ? "D" : "."),
          (((obj.SR & FLAGS6502::I) > 0) ? "I" : "."),
          (((obj.SR & FLAGS6502::Z) > 0) ? "Z" : "."),
          (((obj.SR & FLAGS6502::C) > 0) ? "C" : "."),
          obj.SP
        );
        return os;
      }
    } reg;

    struct DISASSEMBLY
    {
      // 8-bit - Lo-byte
      uint8_t           LowAddress = 0x0;
      // 8-bit - Hi-byte
      uint8_t           HighAddress = 0x0;
      // 8-bit - OpCode
      uint8_t           OpCode = 0x0;
      // string - Mnemonic of OpCode
      std::string       OpCodeString;
      // string - Mnemonic of Address Mode for Operation
      std::string       AddressMode;
      // string - Disassembled Output
      std::string       DisassemblyOutput;

      friend std::ostream &operator<<(std::ostream &os, const DISASSEMBLY& obj)
      {
        fmt::format_to(
          std::ostream_iterator<char>(os),
          "{:02X} {:02X} {:02X} {} {: <37}",
          obj.OpCode, obj.LowAddress, obj.HighAddress,
          obj.OpCodeString, obj.DisassemblyOutput
        );
        return os;
      }
    };

    // Convenience functions to access status register
    uint8_t     GetFlag(FLAGS6502 f);
    void        SetFlag(FLAGS6502 f, bool v);
    std::string DecodeFlag(uint8_t flag);
    uint8_t     DecodeFlag(uint8_t flag, FLAGS6502 f);

  // DISASSEMBLER Functions
  public:
    DISASSEMBLY setDisassembly(uint16_t &addr);
    std::map<uint16_t, DISASSEMBLY> getDisassembly(uint16_t nStart, uint16_t nStop);
    // Produces a map of strings, with keys equivalent to instruction start locations
    // in memory, for the specified address range
    void disassemble(uint16_t addr);
    void disassemble(uint16_t nStart, uint16_t nStop);

  public:
    uint8_t PeekStack();
    void    PokeStack(uint8_t value);
    uint8_t PopStack();
    void    PushStack(uint8_t value);
    void    DumpStack(uint8_t nRows);
    void    DumpStack();
    void    DumpStackAtPointer();

  public:
    void    setRegister(REGISTER6502 f, uint8_t v);
    uint8_t getRegister(REGISTER6502 f);

  private:
    uint8_t cpuspeed = 0; // CPU Speed
    uint8_t GetLogModValue();
    uint16_t GetSleepValue();

  public:
    uint8_t GetCpuSpeed() { return cpuspeed; };
    void SetCpuSpeed(uint8_t nSpeed) { cpuspeed = nSpeed; };

  public:
    uint8_t readMemory(uint16_t a);
    void    writeMemory(uint16_t a, uint8_t d);
    uint8_t readMemoryWithoutCycle(uint16_t a);
    void    writeMemoryWithoutCycle(uint16_t a, uint8_t d);

  public:
    void     addExtraCycle(bool incCycleCount = true);
    void     incrementCycleCount();
    uint16_t incrementProgramCounter();
    uint16_t decrementProgramCounter();
    void     setProgramCounter(uint16_t value);
    uint16_t getProgramCounter();
    uint8_t  incrementStackPointer();
    uint8_t  decrementStackPointer();

  };
}
