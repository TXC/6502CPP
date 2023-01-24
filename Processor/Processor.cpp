#include "Processor.hpp"
//#include "Instructions.hpp"
#include "Types.hpp"
#include "Executioner.hpp"
#include "Bus.hpp"
#include "Logger.hpp"
#include "Exceptions.hpp"

#include <stdexcept>
#include <sstream>
#include <spdlog/spdlog.h>
#if defined(SPDLOG_FMT_EXTERNAL)
#include <fmt/format.h>
#include <fmt/ranges.h>
#else
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ranges.h>
#endif

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

  void Processor::LoadProgram(uint16_t offset, std::string program)
  {
    uint8_t converted[] = {};
    uint16_t pos = offset;

    Logger::log()->info("** Loading Program: {}", program);

    std::stringstream ss;
    ss << program;
    while (!ss.eof())
    {
      std::string b;
      ss >> b;
      converted[pos] = (uint8_t)std::stoul(b, nullptr, 16);
      pos++;
    }
    size_t n = sizeof(converted) / sizeof(converted[0]);
    LoadProgram(offset, converted, n);
  }

  void Processor::LoadProgram(uint16_t offset, std::string program, uint16_t initialProgramCounter)
  {
    LoadProgram(offset, program);

    setProgramCounter(initialProgramCounter);

    uint8_t lo = initialProgramCounter & 0xFF;
    uint8_t hi = (initialProgramCounter >> 8) & 0xFF;

    writeMemoryWithoutCycle(0xFFFC, lo);
    writeMemoryWithoutCycle(0xFFFD, hi);
  }

  void Processor::LoadProgram(uint16_t offset, uint8_t program[], size_t programSize)
  {
    char errorMessage[100];
    size_t ramSize = bus->ram.size();

    if (offset > ramSize)
    {
      throw std::runtime_error(fmt::format(
        "Offset '{:04X}' is larger than memory size '{:d}'",
        offset, ramSize
      ));
    }

    if (programSize > (ramSize + offset))
    {
      throw std::runtime_error(fmt::format(
        "Program size is {:d} bytes, that cannot be larger than memory size {:d} bytes plus offset ${:04X}",
        programSize, ramSize, offset
      ));
    }

    fmt::memory_buffer buffer;

    bus->reset();
    uint16_t pos;
    for (uint16_t i = 0; i < programSize; i++)
    {
      fmt::format_to(std::back_inserter(buffer), "{:02X} ", program[i]);
      pos = (offset + i);
      bus->ram[pos] = program[i];
    }

    Logger::log()->info("** Loading Program: {}", fmt::to_string(buffer));

    reset();
    reg.SP = 0xFF;
  }

  void Processor::LoadProgram(uint16_t offset, uint8_t program[], size_t programSize, uint16_t initialProgramCounter)
  {
    LoadProgram(offset, program, programSize);

    setProgramCounter(initialProgramCounter);
    uint8_t lo = initialProgramCounter & 0xFF;
    uint8_t hi = (initialProgramCounter >> 8) & 0xFF;

    writeMemoryWithoutCycle(0xFFFC, lo);
    writeMemoryWithoutCycle(0xFFFD, hi);
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

    // Reset internal registers
    reg.reset();

    executioner.reset();

    // Reset takes time
    extra_cycles = 0;
    cycle_count = 0;
    operation_cycle = 0;
    jammed = false;

    _previousInterrupt = false;
    TriggerNmi = false;
    TriggerIRQ = false;
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
    if (GetFlag(I) == 1)
    {
      return;
    }

    //pc--;
    decrementProgramCounter();
    executioner.breakOperation(false, 0xFFFE);
    //opcode = readMemory(pc);
    opcode = readMemory(getProgramCounter());
  }

  // A Non-Maskable Interrupt cannot be ignored. It behaves in exactly the
  // same way as a regular IRQ, but reads the new program counter address
  // form location 0xFFFA.
  void Processor::nmi()
  {
    //pc--;
    decrementProgramCounter();
    executioner.breakOperation(false, 0xFFFA);
    opcode = readMemory(getProgramCounter());
  }

  // Perform one clock cycles worth of emulation
  void Processor::tick()
  {
    if (jammed)
    {
#if defined(LOGMODE)
      Logger::log()->error("JAMMED");
#endif
      return;
    }

    // These are per operation
    extra_cycles = 0;
    cycle_count = 0;

    // Read next instruction byte. This 8-bit value is used to index
    // the translation table to get the relevant information about
    // how to implement the instruction

    opcode = readMemory(reg.PC);
    //Logger::log()->info("Processor::tick() PC: 0x{:04X} - OP: 0x{:02X}", reg.PC, opcode);
    try
    {
      //UpdateMemoryMap();

#if defined(LOGMODE)
      disassemble(reg.PC);
      Logger::log()->info(
        "{:>10d}:{:02X} OP: 0x{:02X} / {}:{} {: <17s} {}",
        operation_cycle, cycle_count, opcode, executioner.getInstructionName(opcode), executioner.getAddressModeName(opcode),
        "XXX", reg
      );
#endif

      // Always set the unused status flag bit to 1
      SetFlag(U, true);

      // Increment program counter, we read the opcode byte
      incrementProgramCounter();

      // Get Starting number of cycles
      //cycles = lookup[opcode].cycles;

      // Perform operation incl. fetch of intermmediate
      // data using the required addressing mode
      executioner.execute(opcode);
    }
    catch (const CPU::JammedCPU& e)
    {
      std::cout << "JammedCPU : Executioner::execute() reported a Jammed CPU exception:" << std::endl << e.what() << std::endl;
      return;
    }
    catch (const CPU::IllegalInstruction& e)
    {
      std::cout << "IllegalInstruction : Executioner::execute() reported Illegal Instruction exception:" << std::endl << e.what() << std::endl;
      return;
    }
    catch (const std::runtime_error& e)
    {
      std::cout << "std::runtime_error : Executioner::execute() reported an runtime exception:" << std::endl << e.what() << std::endl;
      return;
    }
    catch (const std::exception& e)
    {
      std::cout << "std::exception : Executioner::execute() reported an exception:" << e.what() << std::endl;
      return;
    }
    catch (...)
    {
      std::cout << "catch_all exception" << std::endl;
      return;
    }


    // Always set the unused status flag bit to 1
    SetFlag(U, true);

#if defined(LOGMODE)
    // This logger dumps every cycle the entire processor state for analysis.
    // This can be used for debugging the emulation, but has little utility
    // during emulation. Its also very slow, so only use if you have to.
    Logger::log()->info(
      "{:>10d}:{:02X} OP: 0x{:02X} / {}:{} {: <17s} {}",
      operation_cycle, cycle_count, opcode, executioner.getInstructionName(opcode), executioner.getAddressModeName(opcode),
      "XXX", reg
    );
#endif

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

    // Increment global clock count - This is actually unused unless logging is enabled
    // but I've kept it in because its a handy watch variable for debugging
    operation_cycle += cycle_count;
  }

  void Processor::setJammed()
  {
    jammed = true;
  }
#pragma endregion EXTERNAL INPUTS

  ///////////////////////////////////////////////////////////////////////////////
#pragma region FLAG FUNCTIONS
// FLAG FUNCTIONS

  // Returns the value of a specific bit of the status register
  uint8_t Processor::GetFlag(FLAGS6502 f)
  {
//#if defined(DEBUG)
//    Logger::log()->debug("GetFlag - Current: 0x{:02X} {} ", reg.SR, DecodeFlag(reg.SR));
//#endif
    //return ((status & f) > 0) ? 1 : 0;
    return ((reg.SR & f) > 0) ? 1 : 0;
  }

  // Sets or clears a specific bit of the status register
  void Processor::SetFlag(FLAGS6502 f, bool v)
  {
    if (v)
    {
      //status |= f;
      reg.SR |= f;
    }
    else
    {
      //status &= ~f;
      reg.SR &= ~f;
    }
  }

  // Get status register as string
  std::string Processor::DecodeFlag(uint8_t flag)
  {
    return fmt::format("{}{}{}{}{}{}{}{}",
      ((flag & N) > 0) ? "N" : ".",
      ((flag & V) > 0) ? "V" : ".",
      ((flag & U) > 0) ? "U" : ".",
      ((flag & B) > 0) ? "B" : ".",
      ((flag & D) > 0) ? "D" : ".",
      ((flag & I) > 0) ? "I" : ".",
      ((flag & Z) > 0) ? "Z" : ".",
      ((flag & C) > 0) ? "C" : "."
    );
  }
  uint8_t Processor::DecodeFlag(uint8_t flag, FLAGS6502 f)
  {
    return ((reg.SR & Z) > 0) ? 1 : 0;
  }

#pragma endregion FLAG FUNCTIONS

  ///////////////////////////////////////////////////////////////////////////////
#pragma region REGISTERS

  void Processor::setRegister(REGISTER6502 f, uint8_t v)
  {
    switch (f)
    {
    case REGISTER6502::AC:
      reg.AC = v;
      break;
    case REGISTER6502::X:
      reg.X = v;
      break;
    case REGISTER6502::Y:
      reg.Y = v;
      break;
    case REGISTER6502::SR:
      reg.SR = v;
      break;
    case REGISTER6502::SP:
      reg.SP = v;
      break;
    case REGISTER6502::PC:
      throw std::invalid_argument("Unable to set Register \"PC\"");
      break;
    }
  }


  uint8_t Processor::getRegister(REGISTER6502 f)
  {
    if (jammed)
    {
      return 0xFF;
    }

    switch (f)
    {
    case REGISTER6502::AC:
      return reg.AC;
      break;
    case REGISTER6502::X:
      return reg.X;
      break;
    case REGISTER6502::Y:
      return reg.Y;
      break;
    case REGISTER6502::SR:
      return reg.SR;
      break;
    case REGISTER6502::SP:
      return reg.SP;
      break;
    case REGISTER6502::PC:
      throw std::invalid_argument("Unable to get Register \"PC\"");
      break;
    default:
      throw std::runtime_error(fmt::format("Invalid register ({:02X})", f));
    }
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


  // Increment Program Counter
  uint16_t Processor::incrementProgramCounter()
  {
    //Logger::log()->debug("PC++ {: >59}", reg);
    reg.PC = (reg.PC + 1) & 0xFFFF;
    return reg.PC;
  }


  // Decrement Program Counter
  uint16_t Processor::decrementProgramCounter()
  {
    reg.PC = (reg.PC - 1) & 0xFFFF;
    return reg.PC;
  }


  // Set Program Counter
  void Processor::setProgramCounter(uint16_t value)
  {
    reg.PC = value & 0xFFFF;
  }


  // Get Program Counter
  uint16_t Processor::getProgramCounter()
  {
    if (jammed)
    {
      return 0xFFFF;
    }

    return reg.PC;
    //return pc;
  }


  // Increment Stack Pointer
  uint8_t Processor::incrementStackPointer()
  {
    reg.SP = (reg.SP + 1) & 0xFF;
    return reg.SP;
  }


  // Decrement Stack Pointer
  uint8_t Processor::decrementStackPointer()
  {
    reg.SP = (reg.SP - 1) & 0xFF;
    //--stkp;
    //return stkp;
    return reg.SP;
  }


  // Peek Stack Pointer (No change on cycle)
  uint8_t Processor::PeekStack()
  {
    //return readMemoryWithoutCycle(stkp + 0x100);
    return readMemoryWithoutCycle(reg.SP + 0x100);
  }


  // Poke Stack Pointer (No change on cycle)
  void Processor::PokeStack(uint8_t value)
  {
    //Logger::log()->info("PokeStack: {:02X} -> ${:04X}", value, (reg.SP + 0x100));
    //writeMemoryWithoutCycle(stkp + 0x100, value);
    writeMemoryWithoutCycle(reg.SP + 0x100, value);
  }


  // Pop Stack (Change on cycle (+2 cycles) & stackpointer)
  uint8_t Processor::PopStack()
  {
    incrementStackPointer();
    //incrementCycleCount();
    return readMemory(reg.SP + 0x100);
  }


  // Pop Stack (Change on cycle & stackpointer)
  void Processor::PushStack(uint8_t value)
  {
    //Logger::log()->info("PushStack: {:02X} -> ${:04X}", value, (reg.SP + 0x100));
    writeMemory(reg.SP + 0x100, value);
    //incrementCycleCount();
    decrementStackPointer();
  }


  // Dump the Nth row from the bottom of the Stack
  void Processor::DumpStack(uint8_t nRows)
  {
    uint16_t offsetStart = (0x200 - (nRows * 0xF)) | 0x100;
    uint16_t offsetStop = 0x01FF;

    bus->dump(offsetStart, offsetStop);
  }

  // Dump the whole Stack region
  void Processor::DumpStack()
  {
    Logger::log()->info("StackPointer is at 0x{:04X}", reg.SP);
    bus->dump(0x100, 0x1FF);
  }


  // Dump the row that StackPointer is located at
  void Processor::DumpStackAtPointer()
  {
    uint16_t stackStart = (reg.SP + 0x100) & 0xFFF0;
    uint16_t stackStop  = (reg.SP + 0x100) | 0x000F;

    if ((stackStart & 0xFF00) != 0x100) {
      stackStart = (stackStart & 0x00FF) + 0x100;
    }
    if ((stackStop & 0xFF00) != 0x100) {
      stackStop = (stackStop & 0x00FF) + 0x100;
    }
    if (stackStop < stackStart) {
      throw std::range_error(fmt::format("Start 0x{:04X} can not be larger than Stop 0x{:04X}", stackStart, stackStop));
    }
    Logger::log()->info("StackPointer @ 0x{:04X} (0x{:04X} - 0x{:04X})", reg.SP, stackStart, stackStop);


    bus->dump(stackStart, stackStop);
  }

#pragma endregion REGISTERS

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


  // Add Extra Cycle (and depending on parameter, increase cycle_count)
  void Processor::addExtraCycle(bool incCycleCount)
  {
    extra_cycles = (extra_cycles + 1);

//#if defined(DEBUG)
//    Logger::log()->debug(
//      "addExtraCycle: {} - incrementCycle: {} - {}",
//      executioner.getOperation(opcode), incCycleCount, reg
//    );
//#endif

    if (incCycleCount)
    {
      incrementCycleCount();
    }
  }


  // Increment Cycle Count
  void Processor::incrementCycleCount()
  {
//#if defined(DEBUG)
//    Logger::log()->debug(
//      "incrementCycleCount: {} - Current Cycle: {} - {}",
//      executioner.getOperation(opcode), cycle_count, reg
//    );
//#endif
    //cycle_count++;
    cycle_count = (cycle_count + 1) & 0xFF;

    _previousInterrupt = _interrupt;
    _interrupt = TriggerNmi || (TriggerIRQ && GetFlag(I) == 0);
  }

#pragma endregion INTERNALS

  ///////////////////////////////////////////////////////////////////////////////
#pragma region DISASSEMBLER FUNCTIONS
// DISASSEMBLER FUNCTIONS

  // This is the disassembly function. Its workings are not required for emulation.
  // It is merely a convenience function to turn the binary instruction code into
  // human readable form. Its included as part of the emulator because it can take
  // advantage of many of the CPUs internal operations to do 
  Processor::DISASSEMBLY Processor::setDisassembly(uint16_t &addr)
  {
    uint8_t value = 0x00,
      lo = 0x00,
      hi = 0x00;

    Processor::DISASSEMBLY CurrentDisassembly;
    // Read instruction, and get its readable name
    uint8_t opcode = bus->read(addr, true);
    std::string opName = executioner.getInstructionName(opcode);
    std::string addrMode = executioner.getAddressModeName(opcode);

    // Prefix line with instruction address
    std::string sInst;
    ++addr;

    // Get operands from desired locations, and form the
    // instruction based upon its addressing mode. These
    // routines mimmick the actual fetch routine of the
    // 6502 in order to get accurate data as part of the
    // instruction

    if (addrMode == "ACC")
    {
      sInst = fmt::format(
        "${:04X}: {} AC {{{}}}",
        addr, opName, addrMode
      );
      //sInst += "AC {ACC}";
    }
    else if (addrMode == "IMP")
    {
      sInst = fmt::format(
        "${:04X}: {} {{{}}}",
        addr, opName, addrMode
      );
      //sInst += "{IMP}";
    }
    else if (addrMode == "IMM")
    {
      lo = bus->read(addr, true);
      ++addr;
      hi = 0x00;
      sInst = fmt::format(
        "${:04X}: {} #${:02X} {{{}}}",
        addr, opName, lo, addrMode
      );
      //sInst += "#$" + hex(value, 2) + " {IMM}";
    }
    else if (addrMode == "ZP0")
    {
      lo = bus->read(addr, true);
      ++addr;
      hi = 0x00;
      sInst = fmt::format(
        "${:04X}: {} ${:02X} {{{}}}",
        addr, opName, lo, addrMode
      );
      //sInst += "$" + hex(lo, 2) + " {ZP0}";
    }
    else if (addrMode == "ZPX")
    {
      lo = bus->read(addr, true);
      ++addr;
      hi = 0x00;
      sInst = fmt::format(
        "${:04X}: {} ${:02X}, X {{{}}}",
        addr, opName, lo, addrMode
      );
      //sInst += "$" + hex(lo, 2) + ", X {ZPX}";
    }
    else if (addrMode == "ZPY")
    {
      lo = bus->read(addr, true);
      ++addr;
      hi = 0x00;
      sInst = fmt::format(
        "${:04X}: {} ${:02X}, Y {{{}}}",
        addr, opName, lo, addrMode
      );
      //sInst += "$" + hex(lo, 2) + ", Y {ZPY}";
    }
    else if (addrMode == "IZX")
    {
      lo = bus->read(addr, true);
      ++addr;
      hi = 0x00;
      sInst = fmt::format(
        "${:04X}: {} (${:02X}, X) {{{}}}",
        addr, opName, lo, addrMode
      );
      //sInst += "($" + hex(lo, 2) + ", X) {IZX}";
    }
    else if (addrMode == "IZY")
    {
      lo = bus->read(addr, true);
      ++addr;
      hi = 0x00;
      sInst = fmt::format(
        "${:04X}: {} (${:02X}), Y {{{}}}",
        addr, opName, lo, addrMode
      );
      //sInst += "($" + hex(lo, 2) + "), Y {IZY}";
    }
    else if (addrMode == "ABS")
    {
      lo = bus->read(addr, true);
      ++addr;
      hi = bus->read(addr, true);
      ++addr;
      sInst = fmt::format(
        "${:04X}: {} ${:04X} {{{}}}",
        addr, opName, (hi << 8) | lo, addrMode
      );
      //sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + " {ABS}";
    }
    else if (addrMode == "ABX")
    {
      lo = bus->read(addr, true);
      ++addr;
      hi = bus->read(addr, true);
      ++addr;
      sInst = fmt::format(
        "${:04X}: {} ${:04X}, X {{{}}}",
        addr, opName, (hi << 8) | lo, addrMode
      );
      //sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + ", X {ABX}";
    }
    else if (addrMode == "ABY")
    {
      lo = bus->read(addr, true);
      ++addr;
      hi = bus->read(addr, true);
      ++addr;
      sInst = fmt::format(
        "${:04X}: {} ${:04X}, Y {{{}}}",
        addr, opName, (hi << 8) | lo, addrMode
      );
      //sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + ", Y {ABY}";
    }
    else if (addrMode == "IND")
    {
      lo = bus->read(addr, true);
      ++addr;
      hi = bus->read(addr, true);
      ++addr;
      sInst = fmt::format(
        "${:04X}: {} (${:04X}) {{{}}}",
        addr, opName, (hi << 8) | lo, addrMode
      );
      //sInst += "($" + hex((uint16_t)(hi << 8) | lo, 4) + ") {IND}";
    }
    else if (addrMode == "REL")
    {
      uint8_t value = bus->read(addr, true);
      ++addr;
      lo = (addr + value) & 0xFF;
      hi = ((addr + value) >> 8) & 0xFF;
      sInst = fmt::format(
        "${:04X}: {} ${:02X} [${:04X}] {{{}}}",
        addr, opName, value, (addr + value), addrMode
      );
      //sInst += "$" + hex(value, 2) + " [$" + hex(addr + value, 4) + "] {REL}";
    }

    CurrentDisassembly = {
      .LowAddress         = lo,
      .HighAddress        = hi,
      .OpCode             = opcode,
      .OpCodeString       = opName,
      .AddressMode        = addrMode,
      .DisassemblyOutput  = sInst
    };

    return CurrentDisassembly;
  }

  // This disassembly function will turn a chunk of binary code into human readable form.
  // See the above function for a more descriptive text
  std::map<uint16_t, Processor::DISASSEMBLY> Processor::getDisassembly(uint16_t nStart, uint16_t nStop)
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
      mapLines[addr] = setDisassembly(++addr);
    }

    return mapLines;
  }

  void Processor::disassemble(uint16_t addr)
  {
#if defined(LOGMODE)
    Processor::DISASSEMBLY CD = setDisassembly(addr);
    Logger::log()->info("{} {}", CD, reg);
#endif
  }

  void Processor::disassemble(uint16_t nStart, uint16_t nStop)
  {
#if defined(LOGMODE)
    uint16_t addr = nStart;
  //Logger::log()->info("disassemble: nStart: ${:04X} nStop: ${:04X}", nStart, nStop);

    Logger::log()->info("\n");
    Logger::log()->info("DISASSEMBLE: ${:04X} - ${:04X}", nStart, nStop);
    Logger::log()->info("OP LO HI OPS DISASSEMBLY");

    while (addr < (uint32_t)nStop)
    {
      Processor::DISASSEMBLY CD = setDisassembly(addr);
      Logger::log()->info("{} {}", CD, reg);
    }
    Logger::log()->info("OP LO HI OPS DISASSEMBLY\n");

#endif
  }

#pragma endregion DISASSEMBLER FUNCTIONS
}
