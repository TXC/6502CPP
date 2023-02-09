#pragma once

#include "cpuExecutioner.hpp"
//#include "Formatters.hpp"
//#include "Logger.hpp"
//#include "BaseBus.hpp"

#include <vector>
#include <map>
#include <memory>
#include <cstdint>
#include <string>
#include <iostream>
#include <spdlog/spdlog.h>
#if defined SPDLOG_FMT_EXTERNAL
#include <fmt/format.h>
#include <fmt/ostream.h>
#else
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ostr.h>
#endif


namespace Processor
{
  // Forward declaration of generic communications bus class to
  // prevent circular inclusions
  class BaseBus;
  class Logger;
  class Executioner;

  // The 6502 Emulation Class. This is it!
  class CPU
  {
  public:
    CPU();
    ~CPU();

    // Linkages
  private:
    // Linkage to the communications bus
    BaseBus* bus = nullptr;
    bool jammed = false;
    // Set to true when an NMI should occur
    bool TriggerNmi = false;
    // Set to true when an IRQ has occurred and is being processed by the CPU
    bool TriggerIRQ = false;

  public:
    // Linkage to the instructions
    Executioner executioner;
    //std::unique_ptr<Executioner> executioner = std::make_unique<Executioner>();

    // Is the instruction byte
    uint8_t  opcode = 0x00;
    // Number of extra cycles that has been added
    uint8_t  extra_cycles = 0;
    // Counts how many cycles the instruction has done
    uint8_t  cycle_count = 0;
    // Operation/Tick cycle
    uint32_t operation_cycle = 0;

    // When true, process interrupt
    bool _previousInterrupt = false;
    // If interrupt shall be triggered before next operation
    bool _interrupt = false;

    // External event functions. In hardware these represent pins that are asserted
    // to produce a change in state.

    void requestNmi() {
      TriggerNmi = true;
    }
    void requestIRQ() {
      TriggerIRQ = true;
    }
    // Reset Interrupt - Forces CPU into known state
    void reset();
    // Interrupt Request - Executes an instruction at a specific location
    void irq();
    // Non-Maskable Interrupt Request - As above, but cannot be disabled
    void nmi();
    // Performs the next step on the processor
    void clock();
    // Set flag that CPU is in a "jammed" state, and requires reset.
    void setJammed() {
      jammed = true;
    };
    bool getJammed() {
      return jammed;
    };

    // Indicates the current instruction has completed by returning true. This is
    // a utility function to enable "step-by-step" execution, without manually 
    // clocking every cycle
    bool complete();

    // Load program
    void loadProgram(uint16_t offset, std::string program);
    void loadProgram(uint16_t offset, std::string program, uint16_t initialProgramCounter);
    void loadProgram(uint16_t offset, uint8_t program[], size_t programSize);
    void loadProgram(uint16_t offset, uint8_t program[], size_t programSize, uint16_t initialProgramCounter);

  // Bus Connectivity
  public:
    // Link this CPU to a communications bus
    void connectBus(BaseBus* n) { bus = n; }

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
          .SR = (0x00 | FLAGS6502::U | FLAGS6502::B)
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
    uint8_t     getFlag(FLAGS6502 f);
    void        setFlag(FLAGS6502 f, bool v);
    std::string decodeFlag(uint8_t flag);
    uint8_t     decodeFlag(uint8_t flag, FLAGS6502 f);

  // DISASSEMBLER Functions
  public:
    DISASSEMBLY setDisassembly(uint16_t &addr);
    std::map<uint16_t, DISASSEMBLY> getDisassembly(uint16_t nStart, uint16_t nStop);
    // Produces a map of strings, with keys equivalent to instruction start locations
    // in memory, for the specified address range
    void        disassemble(uint16_t addr);
    void        disassemble(uint16_t nStart, uint16_t nStop);

  public:
    // Peek Stack Pointer (No change on cycle)
    uint8_t     peekStack() { return readMemoryWithoutCycle(reg.SP + 0x100); };
    // Poke Stack Pointer (No change on cycle)
    void        pokeStack(uint8_t value) { writeMemoryWithoutCycle(reg.SP + 0x100, value); };
    // Pop Stack (Change on cycle (+2 cycles) & stackpointer)
    uint8_t     popStack() { incrementStackPointer(); return readMemory(reg.SP + 0x100); };
    // Pop Stack (Change on cycle & stackpointer)
    void        pushStack(uint8_t value) { writeMemory(reg.SP + 0x100, value); decrementStackPointer(); };


    void        dumpStack(uint8_t nRows);
    void        dumpStack();
    void        dumpStackAtPointer();
    void        setRegister(REGISTER6502 f, uint8_t v);
    void        setRegisterAC(uint8_t v) {
      if (jammed)
      {
        return;
      }

      uint8_t newValue = v & 0xFF;
      reg.AC = newValue;
    };
    void        setRegisterX(uint8_t v) {
      if (jammed)
      {
        return;
      }

      uint8_t newValue = v & 0xFF;
      reg.X = newValue;
    };
    void        setRegisterY(uint8_t v) {
      if (jammed)
      {
        return;
      }

      uint8_t newValue = v & 0xFF;
      reg.Y = newValue;
    };
    void        setRegisterSP(uint8_t v) {
      if (jammed)
      {
        return;
      }

      uint8_t newValue = v & 0xFF;
      reg.SP = newValue;
    };
    void        setRegisterSR(uint8_t v) {
      if (jammed)
      {
        return;
      }

      uint8_t newValue = v & 0xFF;
      reg.SR = newValue;
    };
    void        setProgramCounter(uint16_t v) {
      if (jammed)
      {
        std::cout << "CPU is JAMMED!" << std::endl;
        return;
      }

      uint16_t newValue = v & 0xFFFF;
      reg.PC = newValue;
    };

    uint8_t     getRegister(REGISTER6502 f);
    uint8_t     getRegisterAC() { return jammed ? 0xFF : (reg.AC & 0xFF); };
    uint8_t     getRegisterX() { return jammed ? 0xFF : (reg.X & 0xFF); };
    uint8_t     getRegisterY() { return jammed ? 0xFF : (reg.Y & 0xFF); };
    uint8_t     getRegisterSP() { return jammed ? 0xFF : (reg.SP & 0xFF); };
    uint8_t     getRegisterSR() { return jammed ? 0xFF : (reg.SR & 0xFF); };
    uint16_t    getProgramCounter() { return jammed ? 0xFFFF : (reg.PC & 0xFFFF); };

    uint16_t    incrementProgramCounter() { return reg.PC = ((reg.PC + 1) & 0xFFFF); };
    uint16_t    decrementProgramCounter() { return reg.PC = ((reg.PC - 1) & 0xFFFF); };
    uint8_t     incrementStackPointer() { return reg.SP = ((reg.SP + 1) & 0xFF); };
    uint8_t     decrementStackPointer() { return reg.SP = ((reg.SP - 1) & 0xFF); };

  public:
    int cpuspeed = 0; // CPU Speed
    uint8_t     getLogModValue();
    uint16_t    getSleepValue();
    int         getCpuSpeed() { return cpuspeed; };
    void        setCpuSpeed(int nSpeed) { cpuspeed = nSpeed; };

  public:
    uint8_t     getOpCode() { return opcode; };
    uint8_t     getExtraCycles() { return extra_cycles; };
    uint8_t     getCycleCount() { return cycle_count; };
    uint32_t    getOperationCycleCount() { return operation_cycle; };

  public:
    uint8_t     readMemory(uint16_t a);
    void        writeMemory(uint16_t a, uint8_t d);
    uint8_t     readMemoryWithoutCycle(uint16_t a);
    void        writeMemoryWithoutCycle(uint16_t a, uint8_t d);

  public:
    void        addExtraCycle(bool incCycleCount = true);
    void        incrementCycleCount();
  };
};
