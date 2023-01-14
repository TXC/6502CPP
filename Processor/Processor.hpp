#pragma once

// With little modification, reliance upon the stdlib can
// be removed entirely if required.

// Ths is required for translation table and disassembler. The table
// could be implemented straight up as an array, but I used a vector.
#include <vector>
#include <map>
#include <memory>

// These are required for disassembler. If you dont require disassembly
// then just remove the function.
#include <string>

#include "Executioner.hpp"

namespace CPU
{
  namespace Instructions {
    class AddressMode;
    //class Instruction;
    //class InstructionTable;
  };

  // Forward declaration of generic communications bus class to
  // prevent circular inclusions
  class Bus;
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

  public:
    // Linkage to the instructions
    Executioner executioner;
    //std::unique_ptr<CPU::Executioner> inst;

    // CPU Core registers, exposed as public here for ease of access from external
    // examinors. This is all the 6502 has.
    uint8_t  a = 0x00;         // Accumulator Register
    uint8_t  x = 0x00;         // X Register
    uint8_t  y = 0x00;         // Y Register
    uint8_t  stkp = 0x00;         // Stack Pointer (points to location on bus)
    uint16_t pc = 0x0000;       // Program Counter
    uint8_t  status = 0x00;         // Status Register

    uint8_t  opcode = 0x00;    // Is the instruction byte
    uint8_t  cycle_count = 0;       // Counts how many cycles the instruction has remaining
    uint32_t clock_count = 0;       // A global accumulation of the number of clocks

    bool _previousInterrupt = false;
    bool _interrupt = false;
    bool TriggerNmi = false;
    bool TriggerIRQ = false;

    // External event functions. In hardware these represent pins that are asserted
    // to produce a change in state.
    void reset();   // Reset Interrupt - Forces CPU into known state
    void irq();     // Interrupt Request - Executes an instruction at a specific location
    void nmi();     // Non-Maskable Interrupt Request - As above, but cannot be disabled
    void tick();    // Perform one clock cycle's worth of update

    // Indicates the current instruction has completed by returning true. This is
    // a utility function to enable "step-by-step" execution, without manually 
    // clocking every cycle
    bool complete();

    // Link this CPU to a communications bus
    void ConnectBus(Bus* n) { bus = n; }

    // Dump 16 bytes of RAM 0x???0 -> 0x???F to logfile
    void dumpRam(uint16_t offset);
    // Dump RAM from start offset to end offset to logfile
    void dumpRam(uint16_t offsetStart, uint16_t offsetStop);

    // The status register stores 8 flags. Ive enumerated these here for ease
    // of access. You can access the status register directly since its public.
    // The bits have different interpretations depending upon the context and 
    // instruction being executed.
    enum FLAGS6502
    {
      C = (1 << 0),    // Carry Bit
      Z = (1 << 1),    // Zero
      I = (1 << 2),    // Disable Interrupts
      D = (1 << 3),    // Decimal Mode (unused in this implementation)
      B = (1 << 4),    // Break
      U = (1 << 5),    // Unused
      V = (1 << 6),    // Overflow
      N = (1 << 7),    // Negative
    };

    enum REGISTER6502
    {
      PC = 0,
      AC = 1,
      X = 2,
      Y = 3,
      SR = 4,
      SP = 5,
    };

    uint8_t GetCpuSpeed() { return cpuspeed; };
    void SetCpuSpeed(uint8_t nSpeed) { cpuspeed = nSpeed; };

    // Convenience functions to access status register
    uint8_t     GetFlag(FLAGS6502 f);
    void        SetFlag(FLAGS6502 f, bool v);
    std::string GetFlagString();

    void LoadProgram(uint16_t offset, uint8_t program[], size_t programSize);
    void LoadProgram(uint16_t offset, uint8_t program[], size_t programSize, uint16_t initialProgramCounter);

  public:
    uint8_t PeekStack();
    void    PokeStack(uint8_t value);
    uint8_t PopStack();
    void    PushStack(uint8_t value);

  public:
    void        setRegister(REGISTER6502 f, uint8_t v);
    uint8_t     getRegister(REGISTER6502 f);

  private:
    uint8_t cpuspeed = 0; // CPU Speed    
    uint8_t GetLogModValue();
    uint16_t GetSleepValue();

  public:
    uint8_t readMemory(uint16_t a);
    void    writeMemory(uint16_t a, uint8_t d);
    uint8_t readMemoryWithoutCycle(uint16_t a);
    void    writeMemoryWithoutCycle(uint16_t a, uint8_t d);

  private:
    struct MEMORYMAP
    {
      uint32_t    Offset;
      uint8_t        Pos00;
      uint8_t        Pos01;
      uint8_t        Pos02;
      uint8_t        Pos03;
      uint8_t        Pos04;
      uint8_t        Pos05;
      uint8_t        Pos06;
      uint8_t        Pos07;
      uint8_t        Pos08;
      uint8_t        Pos09;
      uint8_t        Pos0A;
      uint8_t        Pos0B;
      uint8_t        Pos0C;
      uint8_t        Pos0D;
      uint8_t        Pos0E;
      uint8_t        Pos0F;
    };
    std::map<uint32_t, MEMORYMAP> memorymap = {};

    struct DISASSEMBLY
    {
      uint8_t       LowAddress = 0x0;
      uint8_t       HighAddress = 0x0;
      uint8_t       OpCode = 0x0;
      std::string   OpCodeString = nullptr;
      std::string   DisassemblyOutput = nullptr;
    };
    DISASSEMBLY setDisassembly(uint16_t& addr);

  public:
    void     incrementCycleCount();
    uint16_t incrementProgramCounter();
    uint16_t decrementProgramCounter();
    void     setProgramCounter(uint16_t* value);
    void     setProgramCounter(uint16_t value);
    uint16_t getProgramCounter();
    uint8_t  incrementStackPointer();
    uint8_t  decrementStackPointer();

  public:

    void UpdateMemoryMap(uint16_t offset = 0x0000, uint8_t rows = 0xFF, bool clear = true);

    // Produces a map of strings, with keys equivalent to instruction start locations
    // in memory, for the specified address range
    void disassemble(uint16_t addr);
    void disassemble(uint16_t& addr, FILE* fp);
    std::map<uint16_t, DISASSEMBLY> disassemble(uint16_t nStart, uint16_t nStop);
  };
}
