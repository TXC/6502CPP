#include "Processor.hpp"
#include "Operation.hpp"
#include "Executioner.hpp"
#include "Bus.hpp"
#include "Log.hpp"
#include "Instructions/AddressMode.hpp"
#include <stdexcept>


namespace IN = CPU::Instructions;

namespace CPU
{

  // Constructor
  Processor::Processor()
  {
    // Assembles the translation table.
    // The table is one big initialiser list of initialiser lists...
    executioner.ConnectCpu(this);
  }

  Processor::~Processor()
  {
    // Destructor - has nothing to do
  }

  void Processor::LoadProgram(uint16_t offset, uint8_t program[], size_t programSize)
  {
    char errorMessage[100];
    size_t ramSize = bus->ram.size();

    if (offset > ramSize)
    {
      snprintf(
        errorMessage,
        100,
        "Offset '%04X' is larger than memory size '%0zX'",
        offset,
        ramSize
      );
      throw std::runtime_error(errorMessage);
    }

    if (programSize > (ramSize + offset))
    {
      snprintf(
        errorMessage,
        100,
        "Program Size '%0zX' Cannot be larger than Memory Size '%0zX' plus offset '%04X'",
        programSize,
        ramSize,
        offset
      );
      throw std::runtime_error(errorMessage);
    }

    /*
    if (noMemoryReset == false)
    {
        bus->reset();
    }
    */
    bus->reset();
    uint16_t pos;
    for (uint16_t i = 0; i < programSize; i++)
    {
      pos = (offset + i);
      bus->ram[pos] = program[i];
    }
    reset();
  }

  void Processor::LoadProgram(uint16_t offset, uint8_t program[], size_t programSize, uint16_t initialProgramCounter)
  {
    LoadProgram(offset, program, programSize);

    uint8_t lo = initialProgramCounter & 0xFF;
    uint8_t hi = (initialProgramCounter >> 8) & 0xFF;

    writeMemoryWithoutCycle(0xFFFC, lo);
    writeMemoryWithoutCycle(0xFFFD, hi);

    reset();
  }

  ///////////////////////////////////////////////////////////////////////////////
#pragma region BUS CONNECTIVITY
// BUS CONNECTIVITY

// Dump 16 bytes of RAM 0x???0 -> 0x???F to logfile
  void Processor::dumpRam(uint16_t offset)
  {
    if (bus != nullptr)
    {
      bus->dump(offset);
    }
  }
  // Dump RAM from start offset to end offset to logfile
  void Processor::dumpRam(uint16_t offsetStart, uint16_t offsetStop)
  {
    if (bus != nullptr)
    {
      bus->dump(offsetStart, offsetStop);
    }
  }


  // Reads an 8-bit byte from the bus, located at the specified 16-bit address
  uint8_t Processor::readMemory(uint16_t addr)
  {
    // In normal operation "read only" is set to false. This may seem odd. Some
    // devices on the bus may change state when they are read from, and this 
    // is intentional under normal circumstances. However the disassembler will
    // want to read the data at an address without changing the state of the
    // devices on the bus
    incrementCycleCount();
    return bus->read(addr, false);
  }

  // Writes a byte to the bus at the specified address
  void Processor::writeMemory(uint16_t addr, uint8_t data)
  {
    incrementCycleCount();
    bus->write(addr, data);
  }

  // Reads an 8-bit byte from the bus, located at the specified 16-bit address
  uint8_t Processor::readMemoryWithoutCycle(uint16_t addr)
  {
    // In normal operation "read only" is set to false. This may seem odd. Some
    // devices on the bus may change state when they are read from, and this 
    // is intentional under normal circumstances. However the disassembler will
    // want to read the data at an address without changing the state of the
    // devices on the bus
    return bus->read(addr, false);
  }

  // Writes a byte to the bus at the specified address
  void Processor::writeMemoryWithoutCycle(uint16_t addr, uint8_t data)
  {
    bus->write(addr, data);
  }
#pragma endregion BUS CONNECTIVITY

  ///////////////////////////////////////////////////////////////////////////////
#pragma region EXTERNAL INPUTS
// EXTERNAL INPUTS

// Forces the 6502 into a known state. This is hard-wired inside the CPU. The
// registers are set to 0x00, the status register is cleared except for unused
// bit which remains at 1. An absolute address is read from location 0xFFFC
// which contains a second address that the program counter is set to. This 
// allows the programmer to jump to a known and programmable location in the
// memory to start executing from. Typically the programmer would set the value
// at location 0xFFFC at compile time.
  void Processor::reset()
  {
    //UpdateMemoryMap();

    // Get address to set program counter to

    /*
    addr_abs = 0xFFFC;
    uint16_t lo = readMemory(addr_abs + 0);
    uint16_t hi = readMemory(addr_abs + 1);

    // Set it
    pc = (hi << 8) | lo;
    */

    // Reset internal registers
    a = 0;
    x = 0;
    y = 0;
    stkp = 0xFD;
    status = 0x00 | U;

    // Clear internal helper variables
    /*
    addr_rel = 0x0000;
    addr_abs = 0x0000;
    fetched = 0x00;
    */

    executioner.reset();

    // Reset takes time
    cycle_count = 0;
    clock_count = 0;
  }

  // Interrupt requests are a complex operation and only happen if the
  // "disable interrupt" flag is 0. IRQs can happen at any time, but
  // you dont want them to be destructive to the operation of the running 
  // program. Therefore the current instruction is allowed to finish
  // (which I facilitate by doing the whole thing when cycles == 0) and 
  // then the current program counter is stored on the stack. Then the
  // current status register is stored on the stack. When the routine
  // that services the interrupt has finished, the status register
  // and program counter can be restored to how they where before it 
  // occurred. This is impemented by the "RTI" instruction. Once the IRQ
  // has happened, in a similar way to a reset, a programmable address
  // is read form hard coded location 0xFFFE, which is subsequently
  // set to the program counter.
  void Processor::irq()
  {
    // If interrupts aren't allowed
    if (GetFlag(I) != 0)
    {
      return;
    }

    pc--;
    executioner.breakOperation(false, 0xFFFE);
    opcode = readMemory(pc);
  }

  // A Non-Maskable Interrupt cannot be ignored. It behaves in exactly the
  // same way as a regular IRQ, but reads the new program counter address
  // form location 0xFFFA.
  void Processor::nmi()
  {
    pc--;
    executioner.breakOperation(false, 0xFFFA);
    opcode = readMemory(pc);
  }

  // Perform one clock cycles worth of emulation
  void Processor::tick()
  {
    // Read next instruction byte. This 8-bit value is used to index
    // the translation table to get the relevant information about
    // how to implement the instruction
    opcode = readMemory(pc & 0xFFFF);

    //UpdateMemoryMap();
#ifdef LOGMODE
    uint16_t log_pc = pc;
#endif

    // Always set the unused status flag bit to 1
    SetFlag(U, true);

    // Increment program counter, we read the opcode byte
    incrementProgramCounter();

    // Get Starting number of cycles
    //cycles = lookup[opcode].cycles;

    Operation* operation = new Operation(opcode);

    // Perform operation incl. fetch of intermmediate
    // data using the required addressing mode
    uint8_t opcycle = operation->execute(&executioner);

    // The addressmode and opcode may have altered the number
    // of cycles this instruction requires before its completed
    //cycles += (additional_cycle1 & additional_cycle2);

    // Always set the unused status flag bit to 1
    SetFlag(U, true);

#ifdef LOGMODE
    // This logger dumps every cycle the entire processor state for analysis.
    // This can be used for debugging the emulation, but has little utility
    // during emulation. Its also very slow, so only use if you have to.
    if (logfile != nullptr)
    {
      //disassemble(log_pc);
      fprintf(logfile,
        "%10d:%02d OP: 0x%02X / %s:%s PC:%04X %-13s A:%02X X:%02X Y:%02X %s STKP:%02X\n",
        clock_count, cycle_count, opcode, operation->getInstructionName(), operation->getAddressModeName(),
        log_pc, "XXX", a, x, y, GetFlagString().c_str(), stkp);
      fflush(logfile);
    }
#endif

    // Increment global clock count - This is actually unused unless logging is enabled
    // but I've kept it in because its a handy watch variable for debugging
    clock_count++;

    if (_previousInterrupt)
    {
      if (TriggerNmi)
      {
        nmi();
        TriggerNmi = false;
      }
      else if (TriggerIRQ)
      {
        irq();
        TriggerIRQ = false;
      }
    }
  }
#pragma endregion EXTERNAL INPUTS

  ///////////////////////////////////////////////////////////////////////////////
#pragma region FLAG FUNCTIONS
// FLAG FUNCTIONS

// Returns the value of a specific bit of the status register
  uint8_t Processor::GetFlag(FLAGS6502 f)
  {
    return ((status & f) > 0) ? 1 : 0;
  }

  // Sets or clears a specific bit of the status register
  void Processor::SetFlag(FLAGS6502 f, bool v)
  {
    if (v)
      status |= f;
    else
      status &= ~f;
  }

  // Get status register as string
  std::string Processor::GetFlagString()
  {
    return string_format("%s%s%s%s%s%s%s%s",
      GetFlag(N) ? "N" : ".", GetFlag(V) ? "V" : ".", GetFlag(U) ? "U" : ".",
      GetFlag(B) ? "B" : ".", GetFlag(D) ? "D" : ".", GetFlag(I) ? "I" : ".",
      GetFlag(Z) ? "Z" : ".", GetFlag(C) ? "C" : "."
    );
  }
#pragma endregion FLAG FUNCTIONS

  ///////////////////////////////////////////////////////////////////////////////
#pragma region INTERNALS
// INTERNALS

// CPU Internals, like speed control (we don't want it to go to slow or to fast!)
// Output log every: "cycle % GetLogModValue() == 0"
  uint8_t Processor::GetLogModValue()
  {
    switch (cpuspeed)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      return 1;
    case 6:
      return 5;
    case 7:
      return 20;
    case 8:
      return 30;
    case 9:
      return 40;
    case 10:
      return 50;
    default:
      return 5;
    }
  }

  // Clock down Processor a bit, sleep every GetSleepValue() millisecond
  uint16_t Processor::GetSleepValue()
  {
    switch (cpuspeed)
    {
    case 0:
      return 550;
    case 1:
      return 550;
    case 2:
      return 440;
    case 3:
      return 330;
    case 4:
      return 220;
    case 5:
      return 160;
    case 6:
      return 80;
    case 7:
      return 40;
    case 8:
      return 20;
    case 9:
      return 10;
    case 10:
      return 5;
    default:
      return 5;
    }
  }

  // Update Memory Map
  void Processor::UpdateMemoryMap(uint16_t offset, uint8_t rows, bool clear)
  {
    if (clear)
    {
      memorymap.clear();
    }

    uint16_t multiplier = 0;
    for (uint16_t i = offset; i < rows * (offset + 1); i++)
    {
      uint32_t offsetPos = (16 * (uint32_t)multiplier) + (uint32_t)offset;
      memorymap[offsetPos] = {
        //(16 * multiplier) + offset,     // Offset $
        offsetPos,     // Offset $
        readMemoryWithoutCycle(i++), // 0x00
        readMemoryWithoutCycle(i++), // 0x01
        readMemoryWithoutCycle(i++), // 0x02
        readMemoryWithoutCycle(i++), // 0x03
        readMemoryWithoutCycle(i++), // 0x04
        readMemoryWithoutCycle(i++), // 0x05
        readMemoryWithoutCycle(i++), // 0x06
        readMemoryWithoutCycle(i++), // 0x07
        readMemoryWithoutCycle(i++), // 0x08
        readMemoryWithoutCycle(i++), // 0x09
        readMemoryWithoutCycle(i++), // 0x0A
        readMemoryWithoutCycle(i++), // 0x0B
        readMemoryWithoutCycle(i++), // 0x0C
        readMemoryWithoutCycle(i++), // 0x0D
        readMemoryWithoutCycle(i++), // 0x0E
        readMemoryWithoutCycle(i)    // 0x0F
      };
      multiplier++;
    }
  }


  // Increment Cycle Count
  void Processor::incrementCycleCount()
  {
    //cycle_count++;
    cycle_count = (cycle_count + 1) & 0xFF;

    _previousInterrupt = _interrupt;
    _interrupt = TriggerNmi || (TriggerIRQ && GetFlag(I) == 0);
  }


  /*
  void Processor::setRegister(REGISTER6502 f, uint16_t *v)
  {
      switch (f)
      {
          case REGISTER6502::PC:
              setRegister(f, (uint16_t) v);

              break;
          default:
              setRegister(f, (uint8_t) v);
      }
  }
  void Processor::setRegister(REGISTER6502 f, uint16_t v)
  {
      switch (f)
      {
          case REGISTER6502::AC:
              setProgramCounter(v);
              break;
          default:
              setRegister(f, (uint8_t) v);
      }

  }
  */


  void Processor::setRegister(REGISTER6502 f, uint8_t v)
  {
    switch (f)
    {
    case REGISTER6502::PC:
      setProgramCounter((uint16_t)v);
      break;
    case REGISTER6502::AC:
      a = v;
      break;
    case REGISTER6502::X:
      x = v;
      break;
    case REGISTER6502::Y:
      y = v;
      break;
    case REGISTER6502::SR:
      status = v;
      break;
    case REGISTER6502::SP:
      stkp = v;
      break;
    }
  }


  uint8_t Processor::getRegister(REGISTER6502 f)
  {
    switch (f)
    {
      //case REGISTER6502::PC:
      //    return pc;
      //    break;
    case REGISTER6502::AC:
      return a;
      break;
    case REGISTER6502::X:
      return x;
      break;
    case REGISTER6502::Y:
      return y;
      break;
    case REGISTER6502::SR:
      return status;
      break;
    case REGISTER6502::SP:
      return stkp;
      break;
    case REGISTER6502::PC:
      return (uint8_t)getProgramCounter() & 0x00FF;
      break;
    default:
      throw std::runtime_error("Invalid register");
    }
  }


  // Increment Program Counter
  uint16_t Processor::incrementProgramCounter()
  {
    pc = ++pc & 0xFFFF;
    return pc;
  }


  // Decrement Program Counter
  uint16_t Processor::decrementProgramCounter()
  {
    pc = --pc & 0xFFFF;
    return pc;
  }


  // Set Program Counter
  void Processor::setProgramCounter(uint16_t value)
  {
    pc = value & 0xFFFF;
  }


  // Get Program Counter
  uint16_t Processor::getProgramCounter()
  {
    return pc;
  }

  //
  // Increment Stack Pointer
  uint8_t Processor::incrementStackPointer()
  {
    ++stkp;
    return stkp;
  }


  // Decrement Stack Pointer
  uint8_t Processor::decrementStackPointer()
  {
    --stkp;
    return stkp;
  }


  // Peek Stack Pointer (No change on cycle)
  uint8_t Processor::PeekStack()
  {
    return readMemoryWithoutCycle(stkp + 0x100);
  }


  // Poke Stack Pointer (No change on cycle)
  void Processor::PokeStack(uint8_t value)
  {
    writeMemoryWithoutCycle(stkp + 0x100, value);
  }


  // Pop Stack (Change on cycle & stackpointer)
  uint8_t Processor::PopStack()
  {
    incrementStackPointer();
    incrementCycleCount();
    return readMemory(stkp + 0x100);
  }


  // Pop Stack (Change on cycle & stackpointer)
  void Processor::PushStack(uint8_t value)
  {
    writeMemory(stkp + 0x100, value);
    incrementCycleCount();
    decrementStackPointer();
  }

#pragma endregion INTERNALS

  ///////////////////////////////////////////////////////////////////////////////
#pragma region HELPER FUNCTIONS
// HELPER FUNCTIONS

// This is the disassembly function. Its workings are not required for emulation.
// It is merely a convenience function to turn the binary instruction code into
// human readable form. Its included as part of the emulator because it can take
// advantage of many of the CPUs internal operations to do 
  Processor::DISASSEMBLY Processor::setDisassembly(uint16_t& addr)
  {
    uint8_t value = 0x00,
      lo = 0x00,
      hi = 0x00;

    // Read instruction, and get its readable name
    uint8_t opcode = bus->read(addr, true);
    Operation* operation = new Operation(opcode);

    // Prefix line with instruction address
    std::string sInst = string_format(
      "$%04X: %s ",
      addr, operation->getInstructionName()
    );
    addr++;

    Processor::DISASSEMBLY CurrentDisassembly;
    CurrentDisassembly.OpCode = opcode;
    CurrentDisassembly.OpCodeString = operation->getInstructionName();
    CurrentDisassembly.HighAddress = hi;
    CurrentDisassembly.LowAddress = lo;

    // Get operands from desired locations, and form the
    // instruction based upon its addressing mode. These
    // routines mimmick the actual fetch routine of the
    // 6502 in order to get accurate data as part of the
    // instruction

    IN::AddressMode::AddressingModes addrModes;

    switch (operation->getAddressMode())
    {
    case addrModes.Accumulator:
      sInst += "AC {ACC}";
      break;
    case addrModes.Implied:
      sInst += "{IMP}";
      break;
    case addrModes.Immediate:
      value = bus->read(addr, true);
      addr++;
      sInst += "#$" + hex(value, 2) + " {IMM}";
      CurrentDisassembly.LowAddress = value;
      break;
    case addrModes.ZeroPage:
      lo = bus->read(addr, true);
      addr++;
      hi = 0x00;
      sInst += "$" + hex(lo, 2) + " {ZP0}";

      CurrentDisassembly.HighAddress = hi;
      CurrentDisassembly.LowAddress = lo;
      break;
    case addrModes.ZeroPageX:
      lo = bus->read(addr, true);
      addr++;
      hi = 0x00;
      sInst += "$" + hex(lo, 2) + ", X {ZPX}";

      CurrentDisassembly.HighAddress = hi;
      CurrentDisassembly.LowAddress = lo;
      break;
    case addrModes.ZeroPageY:
      lo = bus->read(addr, true);
      addr++;
      hi = 0x00;
      sInst += "$" + hex(lo, 2) + ", Y {ZPY}";

      CurrentDisassembly.HighAddress = hi;
      CurrentDisassembly.LowAddress = lo;
      break;
    case addrModes.IndirectX:
      lo = bus->read(addr, true);
      addr++;
      hi = 0x00;
      sInst += "($" + hex(lo, 2) + ", X) {IZX}";

      CurrentDisassembly.HighAddress = hi;
      CurrentDisassembly.LowAddress = lo;
      break;
    case addrModes.IndirectY:
      lo = bus->read(addr, true);
      addr++;
      hi = 0x00;
      sInst += "($" + hex(lo, 2) + "), Y {IZY}";

      CurrentDisassembly.HighAddress = hi;
      CurrentDisassembly.LowAddress = lo;
      break;
    case addrModes.Absolute:
      lo = bus->read(addr, true);
      addr++;
      hi = bus->read(addr, true);
      addr++;
      sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + " {ABS}";

      CurrentDisassembly.HighAddress = hi;
      CurrentDisassembly.LowAddress = lo;
      break;
    case addrModes.AbsoluteX:
      lo = bus->read(addr, true);
      addr++;
      hi = bus->read(addr, true);
      addr++;
      sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + ", X {ABX}";

      CurrentDisassembly.HighAddress = hi;
      CurrentDisassembly.LowAddress = lo;
      break;
    case addrModes.AbsoluteY:
      lo = bus->read(addr, true);
      addr++;
      hi = bus->read(addr, true);
      addr++;
      sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + ", Y {ABY}";

      CurrentDisassembly.HighAddress = hi;
      CurrentDisassembly.LowAddress = lo;
      break;
    case addrModes.Indirect:
      lo = bus->read(addr, true);
      addr++;
      hi = bus->read(addr, true);
      addr++;
      sInst += "($" + hex((uint16_t)(hi << 8) | lo, 4) + ") {IND}";

      CurrentDisassembly.HighAddress = hi;
      CurrentDisassembly.LowAddress = lo;
      break;
    case addrModes.Relative:
      value = bus->read(addr, true);
      addr++;
      sInst += "$" + hex(value, 2) + " [$" + hex(addr + value, 4) + "] {REL}";

      CurrentDisassembly.LowAddress = (addr + value) & 0xFF;
      CurrentDisassembly.HighAddress = ((addr + value) >> 8) & 0xFF;
      break;
    }

    CurrentDisassembly.DisassemblyOutput = sInst;

    return CurrentDisassembly;
  }

  void Processor::disassemble(uint16_t addr)
  {
#ifdef LOGMODE
    if (CPU::logfile == nullptr)
    {
      return;
    }
    disassemble(addr, CPU::logfile);
#endif
  }

  void Processor::disassemble(uint16_t& addr, FILE* fp)
  {
#ifdef LOGMODE
    Processor::DISASSEMBLY CurrentDisassembly = setDisassembly(addr);
    if (fp == nullptr)
    {
      return;
    }
    if (pc > 0x00)
    {
      fprintf(fp,
        "%04X : %02X %02X %02X %s %-30s A:%02X X:%02X Y:%02X %s STKP:%02X\n",
        pc,
        CurrentDisassembly.OpCode,
        CurrentDisassembly.LowAddress,
        CurrentDisassembly.HighAddress,
        CurrentDisassembly.OpCodeString.c_str(),
        CurrentDisassembly.DisassemblyOutput.c_str(),
        a,
        x,
        y,
        GetFlagString().c_str(),
        stkp
      );
    }
    else
    {
      fprintf(fp,
        "%02X %02X %02X %s %-30s\n",
        CurrentDisassembly.OpCode,
        CurrentDisassembly.LowAddress,
        CurrentDisassembly.HighAddress,
        CurrentDisassembly.OpCodeString.c_str(),
        CurrentDisassembly.DisassemblyOutput.c_str()
      );
    }
    fflush(fp);
#endif
  }

  // This disassembly function will turn a chunk of binary code into human readable form.
  // See the above function for a more descriptive text
  std::map<uint16_t, Processor::DISASSEMBLY> Processor::disassemble(uint16_t nStart, uint16_t nStop)
  {
    uint16_t addr = nStart & 0xFFFF;
    std::map<uint16_t, Processor::DISASSEMBLY> mapLines;

    // Starting at the specified address we read an instruction
    // byte, which in turn yields information from the lookup table
    // as to how many additional bytes we need to read and what the
    // addressing mode is. I need this info to assemble human readable
    // syntax, which is different depending upon the addressing mode

    // As the instruction is decoded, a struct is assembled
    // with the readable output
    while (addr <= (nStop & 0xFFFF))
    {
      // Add the formed string to a std::map, using the instruction's
      // address as the key. This makes it convenient to look for later
      // as the instructions are variable in length, so a straight up
      // incremental index is not sufficient.
      mapLines[addr] = setDisassembly(addr);
    }

    return mapLines;
  }
#pragma endregion HELPER FUNCTIONS
}
