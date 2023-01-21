#include "Executioner.hpp"
//#include "Instructions.hpp"
#include "Processor.hpp"
#include "Logger.hpp"
#include "Types.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <spdlog/spdlog.h>
#ifdef SPDLOG_FMT_EXTERNAL
#include <fmt/format.h>
#else
#include <spdlog/fmt/fmt.h>
#endif
#include <map>
#include <stdexcept>

namespace CPU
{
  Executioner::Executioner()
  {
    loadInstructions();
    opcode = &cpu->opcode;
    cycle_count = &cpu->cycle_count;
    clock_count = &cpu->clock_count;
  }

  Executioner::~Executioner()
  {
    // Destructor - has nothing to do
  }

  void Executioner::reset()
  {
    // Get address to set program counter to
    addr_abs = 0xFFFC;

    uint16_t newPc = (cpu->readMemory(addr_abs + 1) << 8) | cpu->readMemory(addr_abs + 0);

//#ifdef DEBUG
//    Logger::log()->debug("RESET - NEW PC: {:04X} {: >69}", newPc, cpu->reg);
//#endif

    // Set it
    cpu->setProgramCounter(newPc);

    // Clear internal helper variables
    addr_rel = 0x0000;
    addr_abs = 0x0000;
    fetched = 0x00;
  }

  uint8_t Executioner::execute()
  {
    return execute(cpu->opcode);
  }

  uint8_t Executioner::execute(uint8_t op)
  {
    if (!lookup.contains(op)) {
      throw std::runtime_error(fmt::format("Invalid operation ({:02X})", op));
    }

    uint8_t addressModeCycles = 0,
            operationCycles = 0;
    try
    {
#ifdef LOGMODE
      Logger::log()->info("ADDR MODE START    - OP {} {: >53}", getOperation(), cpu->reg);
#endif

      addressModeCycles = (this->*lookup[op].addrmode.op)();

#ifdef LOGMODE
      Logger::log()->info("ADDR MODE FINISHED - OP {} {: >53}", getOperation(), cpu->reg);
#endif

    }
    catch (const std::exception& e)
    {
      std::cout << "std::exception : lookup.addrmode.op["
                << lookup[op].addrmode.name
                << "] reported an exception:"
                << e.what()
                << std::endl;
#ifdef DEBUG
      print_exception(e);
#endif
    }
    catch (...)
    {
      std::cout << "catch_all : lookup.addrmode.op["
                << lookup[op].addrmode.name
                << "] reported an exception:"
                << std::endl;
    }

    try
    {
#ifdef LOGMODE
      Logger::log()->info("OPERATION START    - OP {} {: >53}", getOperation(), cpu->reg);
#endif

      operationCycles = (this->*lookup[op].operate.op)();

#ifdef LOGMODE
      Logger::log()->info("OPERATION FINISHED - OP {} {: >53}", getOperation(), cpu->reg);
#endif

    }
    catch (const std::exception& e)
    {
      std::cout << "std::exception : lookup.operate.op["
                << lookup[op].operate.name
                << "] reported an exception:"
                << e.what()
                << std::endl;
#ifdef DEBUG
      print_exception(e);
#endif
    }
    catch (...)
    {
      std::cout << "catch_all : lookup.operate.op["
                << lookup[op].addrmode.name
                << "] reported an exception:"
                << std::endl;
    }

    return (lookup[op].cycles + (addressModeCycles & operationCycles));
  }

  std::string Executioner::getAddressModeName()
  {
    return getAddressModeName(cpu->opcode);
  }

  std::string Executioner::getAddressModeName(uint8_t op)
  {
    if (!lookup.contains(op)) {
      throw std::runtime_error(fmt::format("Unable to get Address Mode Name - Invalid operation ({:02X})", op));
    }
    return lookup[op].addrmode.name;
  }

  std::string Executioner::getInstructionName()
  {
    return getInstructionName(cpu->opcode);
  }

  std::string Executioner::getInstructionName(uint8_t op)
  {
    if (!lookup.contains(op)) {
      throw std::runtime_error(fmt::format("Unable to get Instruction Name - Invalid operation ({:02X})", op));
    }
    return lookup[op].operate.name;
  }

  std::string Executioner::getOperation()
  {
    return getOperation(cpu->opcode);
  }

  std::string Executioner::getOperation(uint8_t op)
  {
    if (!lookup.contains(op)) {
      throw std::runtime_error(fmt::format("Unable to get Operation - Invalid operation ({:02X})", op));
    }

    return fmt::format("{}:{} [{:02X}]", lookup[op].operate.name, lookup[op].addrmode.name, op);
  }


  // This function sources the data used by the instruction into 
  // a convenient numeric variable. Some instructions dont have to 
  // fetch data as the source is implied by the instruction. For example
  // "INX" increments the X register. There is no additional data
  // required. For all other addressing modes, the data resides at 
  // the location held within addr_abs, so it is read from there. 
  // Immediate adress mode exploits this slightly, as that has
  // set addr_abs = pc + 1, so it fetches the data from the
  // next byte for example "LDA $FF" just loads the accumulator with
  // 256, i.e. no far reaching memory fetch is required. "fetched"
  // is a variable global to the CPU, and is set by calling this 
  // function. It also returns it for convenience.
  uint8_t Executioner::fetch()
  {
    std::vector<std::string> ignoredAddrModes;
    ignoredAddrModes.push_back("ACC");
    ignoredAddrModes.push_back("IMP");

    if (!in_array<std::string>(getAddressModeName(), ignoredAddrModes))
    {
      fetched = cpu->readMemory(addr_abs);
#ifdef DEBUG
      Logger::log()->debug(
        "{}: FETCHED 0x{:04X} FROM ${:04X} ",
        getInstructionName(), fetched, addr_abs
      );
#endif
    }
    return fetched;
  }

  ///////////////////////////////////////////////////////////////////////////////
#pragma region ADDRESSING MODES
// ADDRESSING MODES

// The 6502 can address between 0x0000 - 0xFFFF. The high byte is often referred
// to as the "page", and the low byte is the offset into that page. This implies
// there are 256 pages, each containing 256 bytes.
//
// Several addressing modes have the potential to require an additional clock
// cycle if they cross a page boundary. This is combined with several instructions
// that enable this additional clock cycle. So each addressing function returns
// a flag saying it has potential, as does each instruction. If both instruction
// and address function return 1, then an additional clock cycle is required.

// Address Mode: Accumulator
// Operand is always AC
  uint8_t Executioner::ACC()
  {
    //cpu->readMemory(pc);
    //cpu->incrementCycleCount();
    fetched = cpu->getRegister(cpu->AC);

#ifdef DEBUG
    Logger::log()->debug(
      "OP {} {: >75}",
      getOperation(), cpu->reg
    );
#endif
    return 0;
  }

  // Address Mode: Implied
  // There is no additional data required for this instruction. The instruction
  // does something very simple like like sets a status bit. However, we will
  // target the accumulator, for instructions like PHA
  uint8_t Executioner::IMP()
  {
    //fetched = a;
    //cpu->incrementCycleCount();
    fetched = 0x0;
//#ifdef DEBUG
//    Logger::log()->debug(
//      "OP {} {: >75}",
//      getOperation(), fetched, cpu->reg
//    );
//#endif
    return 0;
  }

  // Address Mode: Immediate
  // The instruction expects the next byte to be used as a value, so we'll prep
  // the read address to point to the next byte
  uint8_t Executioner::IMM()
  {
    addr_abs = cpu->getProgramCounter();
    cpu->incrementProgramCounter();
    //cpu->incrementCycleCount();
#ifdef DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} {: >57}",
      getOperation(), addr_abs, cpu->reg
    );
#endif
    return 0;
  }

  // Address Mode: Zero Page
  // To save program bytes, zero page addressing allows you to absolutely address
  // a location in first 0xFF bytes of address range. Clearly this only requires
  // one byte instead of the usual two.
  uint8_t Executioner::ZP0()
  {
#ifdef DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} {: >58}",
      getOperation(), addr_abs, cpu->reg
    );
#endif

    addr_abs = cpu->readMemory(cpu->getProgramCounter());

    cpu->incrementProgramCounter();
    addr_abs &= 0x00FF;

#ifdef DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} {: >58}",
      getOperation(), addr_abs, cpu->reg
    );
#endif
    return 0;
  }

  // Address Mode: Zero Page with X Offset
  // Fundamentally the same as Zero Page addressing, but the contents of the X Register
  // is added to the supplied single byte address. This is useful for iterating through
  // ranges within the first page.
  uint8_t Executioner::ZPX()
  {
#ifdef DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} {: >57}",
      getOperation(), addr_abs, cpu->reg
    );
#endif

    addr_abs = cpu->readMemory(cpu->getProgramCounter());
    //cpu->readMemory(addr_abs);
    cpu->incrementCycleCount();

    addr_abs += cpu->getRegister(cpu->X);

    cpu->incrementProgramCounter();
    addr_abs &= 0x00FF;

#ifdef DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} {: >57}",
      getOperation(), addr_abs, cpu->reg
    );
#endif
    return 0;
  }

  // Address Mode: Zero Page with Y Offset
  // Same as above but uses Y Register for offset
  uint8_t Executioner::ZPY()
  {
#ifdef DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} {: >57}",
      getOperation(), addr_abs, cpu->reg
    );
#endif
    addr_abs = cpu->readMemory(cpu->getProgramCounter());
    //cpu->readMemory(addr_abs);
    cpu->incrementCycleCount();

    addr_abs += cpu->getRegister(cpu->Y);

    cpu->incrementProgramCounter();
    addr_abs &= 0x00FF;

#ifdef DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} {: >57}",
      getOperation(), addr_abs, cpu->reg
    );
#endif
    return 0;
  }

  // Address Mode: Relative
  // This address mode is exclusive to branch instructions. The address
  // must reside within -128 to +127 of the branch instruction, i.e.
  // you cant directly branch to any address in the addressable range.
  uint8_t Executioner::REL()
  {
    addr_rel = cpu->readMemory(cpu->getProgramCounter());

#ifdef DEBUG
    Logger::log()->debug(
      "OP {} - addr_rel: {:04X} {: >57}",
      getOperation(), addr_rel, cpu->reg
    );
#endif

    uint16_t a2 = (addr_rel + 1) & 0xFFFF;
#ifdef LOGMODE
    cpu->dumpRam(cpu->getProgramCounter());
#endif
    //cpu->incrementProgramCounter();
    if (addr_rel & 0x80)
    {
#ifdef DEBUG
      Logger::log()->debug(
        "OP {} - addr_rel [PB]: {:04X} -> {:04X} {: >40}",
        getOperation(), addr_rel, addr_rel | 0xFF00, cpu->reg
      );
#endif
      //addr_rel = a2 - ((addr_rel ^ 0xFF) + 1);
      //addr_rel |= 0xFF00;
      addr_rel = (a2 | 0xFF00);
      //cpu->incrementCycleCount();
    }
    else
    {
      addr_rel = a2;
    }
#ifdef DEBUG
    Logger::log()->debug(
      "OP {} - addr_rel: {:04X} {: >57}",
      getOperation(), addr_rel, cpu->reg
    );
#endif
    return 0;
  }

  // Address Mode: Absolute 
  // A full 16-bit address is loaded and used
  uint8_t Executioner::ABS()
  {
#ifdef DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} {: >57}",
      getOperation().c_str(), addr_abs, cpu->reg
    );
#endif

    uint16_t lo = cpu->readMemory(cpu->getProgramCounter());
    cpu->incrementProgramCounter();

    uint16_t hi = cpu->readMemory(cpu->getProgramCounter());
    cpu->incrementProgramCounter();

    addr_abs = (hi << 8) | lo;

#ifdef DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} HI:{:02X} LO:{:02X} {: >45}",
      getOperation(), addr_abs, hi, lo, cpu->reg
    );
#endif
    return 0;
  }

  // Address Mode: Absolute with X Offset
  // Fundamentally the same as absolute addressing, but the contents of the X Register
  // is added to the supplied two byte address. If the resulting address changes
  // the page, an additional clock cycle is required
  uint8_t Executioner::ABX()
  {
#ifdef DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} {: >57}",
      getOperation(), addr_abs, cpu->reg
    );
#endif
    uint16_t lo = cpu->readMemory(cpu->getProgramCounter());
    cpu->incrementProgramCounter();

    uint16_t hi = cpu->readMemory(cpu->getProgramCounter());
    cpu->incrementProgramCounter();


    addr_abs = (hi << 8) | lo;
    addr_abs += cpu->getRegister(cpu->X);

#ifdef DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} HI:{:02X} LO:{:02X} {: >45}",
      getOperation(), addr_abs, hi, lo, cpu->reg
    );
#endif

    std::vector<uint8_t> ignoredOpCodes;
    ignoredOpCodes.push_back(0x1E);
    ignoredOpCodes.push_back(0xDE);
    ignoredOpCodes.push_back(0xFE);
    ignoredOpCodes.push_back(0x5E);
    ignoredOpCodes.push_back(0x3E);
    ignoredOpCodes.push_back(0x7E);
    ignoredOpCodes.push_back(0x9D);

    if ((addr_abs & 0xFF00) != (hi << 8) && !in_array<uint8_t>(cpu->opcode, ignoredOpCodes))
    {
      //cpu->incrementCycleCount();
      cpu->addExtraCycle();
#ifdef DEBUG
      Logger::log()->debug(
        "OP {} - addr_abs (Page Boundary): {:04X} HI:{:02X} LO:{:02X} {: >40}",
        getOperation(), addr_abs, hi, lo, cpu->reg
      );
#endif
    }
#ifdef LOGMODE
    else if (in_array<uint8_t>(cpu->opcode, ignoredOpCodes))
    {
      Logger::log()->info("OP {} - Caught ignored OPCode", getOperation());
    }
#endif
    return 0;
  }

  // Address Mode: Absolute with Y Offset
  // Fundamentally the same as absolute addressing, but the contents of the Y Register
  // is added to the supplied two byte address. If the resulting address changes
  // the page, an additional clock cycle is required
  uint8_t Executioner::ABY()
  {
#ifdef DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} {: >57}",
      getOperation(), addr_abs, cpu->reg
    );
#endif

    uint16_t lo = cpu->readMemory(cpu->getProgramCounter());
    cpu->incrementProgramCounter();

    uint16_t hi = cpu->readMemory(cpu->getProgramCounter());
    cpu->incrementProgramCounter();

    addr_abs = (hi << 8) | lo;
    addr_abs += cpu->getRegister(cpu->Y);

#ifdef DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} HI:{:02X} LO:{:02X} {: >45}",
      getOperation(), addr_abs, hi, lo, cpu->reg
    );
#endif

    std::vector<uint8_t> ignoredOpCodes;
    ignoredOpCodes.push_back(0x99);
    if ((addr_abs & 0xFF00) != (hi << 8) && !in_array<uint8_t>(cpu->opcode, ignoredOpCodes))
    {
      //cpu->incrementCycleCount();
      cpu->addExtraCycle();
#ifdef DEBUG
      Logger::log()->debug(
        "OP {} - addr_abs (Page Boundary): {:04X} HI:{:02X} LO:{:02X} {: >40}",
        getOperation(), addr_abs, hi, lo, cpu->reg
      );
#endif
    }
#ifdef LOGMODE
    else if (in_array<uint8_t>(cpu->opcode, ignoredOpCodes))
    {
      Logger::log()->info("OP {} - Caught ignored OPCode", getOperation());
    }
#endif
    return 0;
  }

  // Note: The next 3 address modes use indirection (aka Pointers!)

  // Address Mode: Indirect
  // The supplied 16-bit address is read to get the actual 16-bit address. This is
  // instruction is unusual in that it has a bug in the hardware! To emulate its
  // function accurately, we also need to emulate this bug. If the low byte of the
  // supplied address is 0xFF, then to read the high byte of the actual address
  // we need to cross a page boundary. This doesnt actually work on the chip as 
  // designed, instead it wraps back around in the same page, yielding an 
  // invalid actual address
  uint8_t Executioner::IND()
  {
#ifdef DEBUG
    Logger::log()->debug("OP {} {: >74}", getOperation(), cpu->reg);
#endif

    uint16_t ptr_lo = cpu->readMemory(cpu->getProgramCounter());
    cpu->incrementProgramCounter();
    uint16_t ptr_hi = cpu->readMemory(cpu->getProgramCounter());
    cpu->incrementProgramCounter();

    uint16_t ptr = (ptr_hi << 8) | ptr_lo;

#ifndef EMULATE65C02
    // Simulate page boundary hardware bug in 6502 (fixed in 65C02)
    // The indirect jump instruction does not increment the
    // page address when the indirect pointer crosses a
    // page boundary. JMP ($xxFF) will fetch the address
    // from $xxFF and $xx00.
    if (ptr_lo == 0x00FF)
    {
      addr_abs = (cpu->readMemory(ptr & 0xFF00) << 8) | cpu->readMemory(ptr + 0);
#ifdef DEBUG
      Logger::log()->debug(
        "OP {} HW BUG - addr_abs: {:04X} PTR:{:04X} HI:{:02X} LO:{:02X} {: >40}",
        getOperation(), addr_abs, ptr, 0x00, ptr_lo, cpu->reg
      );
#endif
    }
    else // Behave normally
    {
#endif
      addr_abs = (cpu->readMemory(ptr + 1) << 8) | cpu->readMemory(ptr + 0);
#ifdef DEBUG
      Logger::log()->debug(
        "OP {} addr_abs: {:04X} PTR:{:04X} HI:{:02X} LO:{:02X} {: >40}",
        getOperation(), addr_abs, ptr, ptr_hi, ptr_lo, cpu->reg
      );
#endif
#ifndef EMULATE65C02
    }
#endif
    return 0;
  }

  // Address Mode: Indirect X
  // The supplied 8-bit address is offset by X Register to index
  // a location in page 0x00. The actual 16-bit address is read 
  // from this location
  uint8_t Executioner::IZX()
  {
    uint16_t t = cpu->readMemory(cpu->getProgramCounter());

#ifdef DEBUG
    Logger::log()->debug(
      "OP {} - t: 0x{:02X} {: >64}",
      getOperation(), t, cpu->reg
    );
#endif

    cpu->incrementProgramCounter();
    //cpu->readMemory(t);
    cpu->incrementCycleCount();

    uint8_t x = cpu->getRegister(cpu->X);
    uint16_t lo = cpu->readMemory((uint16_t)(t + (uint16_t)(size_t)x) & 0x00FF);
    uint16_t hi = cpu->readMemory((uint16_t)(t + (uint16_t)(size_t)x + 1) & 0x00FF);

    addr_abs = (hi << 8) | lo;

#ifdef DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} HI:{:02X} LO:{:02X} {: >45}",
      getOperation(), addr_abs, hi, lo, cpu->reg
    );
#endif
    return 0;
  }

  // Address Mode: Indirect Y
  // The supplied 8-bit address indexes a location in page 0x00. From 
  // here the actual 16-bit address is read, and the contents of
  // Y Register is added to it to offset it. If the offset causes a
  // change in page then an additional clock cycle is required.
  uint8_t Executioner::IZY()
  {
    uint16_t t = cpu->readMemory(cpu->getProgramCounter());

#ifdef DEBUG
    Logger::log()->debug(
      "OP {} - t: 0x{:02X} {: >64}",
      getOperation(), t, cpu->reg
    );
#endif

    cpu->incrementProgramCounter();
    //cpu->incrementCycleCount();

    uint16_t lo = cpu->readMemory(t & 0x00FF);
    uint16_t hi = cpu->readMemory((t + 1) & 0x00FF);

    addr_abs = (hi << 8) | lo;
    addr_abs += cpu->getRegister(cpu->Y);

#ifdef DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} HI:{:02X} LO:{:02X} {: >45}",
      getOperation(), addr_abs, hi, lo, cpu->reg
    );
#endif

    std::vector<uint8_t> ignoredOpCodes;
    ignoredOpCodes.push_back(0x91);
    if ((addr_abs & 0xFF00) != (hi << 8) && !in_array<uint8_t>(cpu->opcode, ignoredOpCodes))
    {
      //cpu->incrementCycleCount();
      cpu->addExtraCycle();
#ifdef DEBUG
      Logger::log()->debug(
        "OP {} - addr_abs (Page Boundary): {:04X} HI:{:02X} LO:{:02X} {: >40}",
        getOperation(), addr_abs, hi, lo, cpu->reg
      );
#endif
    }
#ifdef LOGMODE
    else if (in_array<uint8_t>(cpu->opcode, ignoredOpCodes))
    {
      Logger::log()->info("OP {} - Caught ignored OPCode", getOperation());
    }
#endif
    return 0;
  }
#pragma endregion ADDRESSING MODES

  ///////////////////////////////////////////////////////////////////////////////
#pragma region COMMON OPERATION

// Performs the different branch operations.
// Based on performBranch is true or not
  void Executioner::branchOperation(bool performBranch)
  {
    if (performBranch == false)
    {
#ifdef DEBUG
      Logger::log()->debug(
        "OP {} - {:02X} - Not Branching",
        getOperation(), cpu->opcode
      );
#endif
      //cpu->incrementCycleCount();
      cpu->incrementProgramCounter();
      return;
    }

    uint16_t pc = cpu->getProgramCounter();

    addr_abs = (pc + addr_rel) & 0xFFFF;

#ifdef DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} - addr_rel: {:04X} - PC: {:04X} - REL + PC: {:06X}",
      getOperation(), addr_abs, addr_rel, pc, (pc + addr_rel)
    );
#endif

    if ((addr_abs & 0xFF00) != (pc & 0xFF00))
    {
      cpu->incrementCycleCount();
#ifdef DEBUG
      Logger::log()->debug(
        "OP {} [PB] - addr_abs: {:04X} - addr_rel: {:04X} - PC: {:04X}",
        getOperation(), addr_abs, addr_rel, pc
      );
#endif
    }

    //pc = addr_abs & 0xFFFF;
    cpu->setProgramCounter(addr_abs);
    //cpu->readMemory(addr_abs);
    cpu->incrementCycleCount();
  }


  // The BRK routine. Called when a BRK occurs.
  // Also called from NMI/IRQ operations
  void Executioner::breakOperation(bool isBreak, uint16_t vector)
  {
    cpu->incrementProgramCounter();
    cpu->incrementCycleCount();

    uint16_t pc = cpu->getProgramCounter();
#ifdef LOGMODE
    cpu->dumpRam(cpu->getProgramCounter()-1);
#endif

    cpu->PokeStack((pc >> 8) & 0x00FF);
    cpu->decrementStackPointer();
    cpu->incrementCycleCount();

    cpu->PokeStack(pc & 0x00FF);
    cpu->decrementStackPointer();
    cpu->incrementCycleCount();

    if (isBreak)
    {
      cpu->SetFlag(cpu->B, true);
    }
    else
    {
      cpu->SetFlag(cpu->B, false);
    }

    cpu->PokeStack(cpu->getRegister(cpu->SR));
    cpu->decrementStackPointer();
    cpu->incrementCycleCount();

    cpu->SetFlag(cpu->I, true);
#ifdef EMULATE65C02
    cpu->SetFlag(cpu->D, true);
#endif

    uint16_t newPc = (cpu->readMemory(vector + 1) << 8) | cpu->readMemory(vector);

//#ifdef DEBUG
//    Logger::log()->debug("OP {} - NEW PC: {:04X} {: >59}", getOperation(), newPc, cpu->reg);
//#endif

#ifdef LOGMODE
    cpu->DumpStackAtPointer();
#endif

    cpu->setProgramCounter(newPc);

    cpu->_previousInterrupt = false;
  }
#pragma endregion COMMON OPERATION

  ///////////////////////////////////////////////////////////////////////////////
#pragma region INSTRUCTION IMPLEMENTATIONS
// INSTRUCTION IMPLEMENTATIONS

// Note: Ive started with the two most complicated instructions to emulate, which
// ironically is addition and subtraction! Ive tried to include a detailed 
// explanation as to why they are so complex, yet so fundamental. Im also NOT
// going to do this through the explanation of 1 and 2's complement.

// Instruction: Add with Carry In
// Function:    A = A + M + C
// Flags Out:   C, V, N, Z
//
// Explanation:
// The purpose of this function is to add a value to the accumulator and a carry bit. If
// the result is > 255 there is an overflow setting the carry bit. Ths allows you to
// chain together ADC instructions to add numbers larger than 8-bits. This in itself is
// simple, however the 6502 supports the concepts of Negativity/Positivity and Signed Overflow.
//
// 10000100 = 128 + 4 = 132 in normal circumstances, we know this as unsigned and it allows
// us to represent numbers between 0 and 255 (given 8 bits). The 6502 can also interpret 
// this word as something else if we assume those 8 bits represent the range -128 to +127,
// i.e. it has become signed.
//
// Since 132 > 127, it effectively wraps around, through -128, to -124. This wraparound is
// called overflow, and this is a useful to know as it indicates that the calculation has
// gone outside the permissable range, and therefore no longer makes numeric sense.
//
// Note the implementation of ADD is the same in binary, this is just about how the numbers
// are represented, so the word 10000100 can be both -124 and 132 depending upon the 
// context the programming is using it in. We can prove this!
//
//  10000100 =  132  or  -124
// +00010001 = + 17      + 17
//  ========    ===       ===     See, both are valid additions, but our interpretation of
//  10010101 =  149  or  -107     the context changes the value, not the hardware!
//
// In principle under the -128 to 127 range:
// 10000000 = -128, 11111111 = -1, 00000000 = 0, 00000000 = +1, 01111111 = +127
// therefore negative numbers have the most significant set, positive numbers do not
//
// To assist us, the 6502 can set the overflow flag, if the result of the addition has
// wrapped around. V <- ~(A^M) & A^(A+M+C) :D lol, let's work out why!
//
// Let's suppose we have A = 30, M = 10 and C = 0
//          A = 30 = 00011110
//          M = 10 = 00001010+
//     RESULT = 40 = 00101000
//
// Here we have not gone out of range. The resulting significant bit has not changed.
// So let's make a truth table to understand when overflow has occurred. Here I take
// the MSB of each component, where R is RESULT.
//
// A  M  R | V | A^R | A^M |~(A^M) | 
// 0  0  0 | 0 |  0  |  0  |   1   |
// 0  0  1 | 1 |  1  |  0  |   1   |
// 0  1  0 | 0 |  0  |  1  |   0   |
// 0  1  1 | 0 |  1  |  1  |   0   |  so V = ~(A^M) & (A^R)
// 1  0  0 | 0 |  1  |  1  |   0   |
// 1  0  1 | 0 |  0  |  1  |   0   |
// 1  1  0 | 1 |  1  |  0  |   1   |
// 1  1  1 | 0 |  0  |  0  |   1   |
//
// We can see how the above equation calculates V, based on A, M and R. V was chosen
// based on the following hypothesis:
//       Positive Number + Positive Number = Negative Result -> Overflow
//       Negative Number + Negative Number = Positive Result -> Overflow
//       Positive Number + Negative Number = Either Result -> Cannot Overflow
//       Positive Number + Positive Number = Positive Result -> OK! No Overflow
//       Negative Number + Negative Number = Negative Result -> OK! NO Overflow
  uint8_t Executioner::ADC()
  {
    // Grab the data that we are adding to the accumulator
    fetch();

    uint8_t current = cpu->getRegister(cpu->AC);
#ifdef DECIMAL_MODE
    if (cpu->GetFlag(cpu->D))
    {
      uint8_t d0 = (fetched & 0x0F) + (current & 0x0F) + (uint8_t)cpu->GetFlag(cpu->C);
      uint8_t d1 = (fetched >> 4) + (current >> 4) + (d0 > 9 ? 1 : 0);

      temp = d0 % 10 | (d1 % 10 << 4);

      cpu->SetFlag(cpu->C, d1 > 9);
    }
    else
    {
#endif
      // Add is performed in 16-bit domain for emulation to capture any
      // carry bit, which will exist in bit 8 of the 16-bit word
      temp = (uint16_t)current + (uint16_t)fetched + (uint16_t)cpu->GetFlag(cpu->C);

      // The signed Overflow flag is set based on all that up there! :D
      cpu->SetFlag(cpu->V, (~((uint16_t)fetched ^ (uint16_t)current) & ((uint16_t)temp ^ (uint16_t)current)) & 0x80);

      //Logger::log->info("A: 0x{:04X} - Fetched: 0x{:04X} - TEMP: 0x{:04X}", a, fetched, temp);

      cpu->SetFlag(cpu->C, temp > 255);
      temp = temp & 0x00FF;
#ifdef DECIMAL_MODE
    }
#endif

    // The Zero flag is set if the result is 0
    cpu->SetFlag(cpu->Z, (temp & 0x00FF) == 0);

    // The negative flag is set to the most significant bit of the result
    cpu->SetFlag(cpu->N, (temp & 0x80) > 0);

    // Load the result into the accumulator (it's 8-bit dont forget!)
    cpu->setRegister(cpu->AC, (uint8_t)(temp & 0x00FF));

    // This instruction has the potential to require an additional clock cycle
    return 1;
  }


  // Instruction: Subtraction with Borrow In
  // Function:    A = A - M - (1 - C)
  // Flags Out:   C, V, N, Z
  //
  // Explanation:
  // Given the explanation for ADC above, we can reorganise our data
  // to use the same computation for addition, for subtraction by multiplying
  // the data by -1, i.e. make it negative
  //
  // A = A - M - (1 - C)  ->  A = A + -1 * (M - (1 - C))  ->  A = A + (-M + 1 + C)
  //
  // To make a signed positive number negative, we can invert the bits and add 1
  // (OK, I lied, a little bit of 1 and 2s complement :P)
  //
  //  5 = 00000101
  // -5 = 11111010 + 00000001 = 11111011 (or 251 in our 0 to 255 range)
  //
  // The range is actually unimportant, because if I take the value 15, and add 251
  // to it, given we wrap around at 256, the result is 10, so it has effectively 
  // subtracted 5, which was the original intention. (15 + 251) % 256 = 10
  //
  // Note that the equation above used (1-C), but this got converted to + 1 + C.
  // This means we already have the +1, so all we need to do is invert the bits
  // of M, the data(!) therfore we can simply add, exactly the same way we did 
  // before.
  uint8_t Executioner::SBC()
  {
    fetch();

    uint8_t current = cpu->getRegister(cpu->AC);
    uint16_t value = 0x00;
#ifdef LOGMODE
    Logger::log()->info("OP {} - REGISTERS: {: >61}", getOperation(), cpu->reg);
#endif

#ifdef DECIMAL_MODE
    if (cpu->GetFlag(cpu->D))
    {
      int8_t d0 = (current & 0x0F) - (fetched & 0x0F) - (cpu->GetFlag(cpu->C) ? 0 : 1);
      int8_t d1 = (current >> 4) - (fetched >> 4) - (d0 < 0 ? 1 : 0);

      value = (d0 < 0 ? 10 + d0 : d0) | ((d1 < 0 ? 10 + d1 : d1) << 4);
      cpu->setRegister(cpu->AC, (uint8_t)(value & 0xFF));

      cpu->SetFlag(cpu->C, d1 < 0);
    }
    else
    {
#endif
      // Operating in 16-bit domain to capture carry out

      // We can invert the bottom 8 bits with bitwise xor
      uint16_t bottom = ((uint16_t)fetched) ^ 0x00FF;

      // Notice this is exactly the same as addition from here!
      value = (uint16_t)current + bottom + (uint16_t)cpu->GetFlag(cpu->C);

      cpu->SetFlag(cpu->V, (value ^ (uint16_t)current) & (value ^ bottom) & 0x0080);
      cpu->SetFlag(cpu->C, value & 0xFF00);

      cpu->setRegister(cpu->AC, (uint8_t)(value & 0xFF));
#ifdef DECIMAL_MODE
    }
#endif

    cpu->SetFlag(cpu->Z, (value & 0x00FF) == 0);
    cpu->SetFlag(cpu->N, value & 0x0080);

    return 1;
  }


  // OK! Complicated operations are done! the following are much simpler
  // and conventional. The typical order of events is:
  // 1) Fetch the data you are working with
  // 2) Perform calculation
  // 3) Store the result in desired place
  // 4) Set Flags of the status register
  // 5) Return if instruction has potential to require additional 
  //    clock cycle

  // Instruction: Bitwise Logic AND
  // Function:    A = A & M
  // Flags Out:   N, Z
  uint8_t Executioner::AND()
  {
    fetch();
    uint8_t value = (cpu->getRegister(cpu->AC) & fetched);
    cpu->setRegister(cpu->AC, value);
    cpu->SetFlag(cpu->Z, value == 0x00);
    cpu->SetFlag(cpu->N, value & 0x80);
    return 1;
  }


  // Instruction: Arithmetic Shift Left
  // Function:    A = C <- (A << 1) <- 0
  // Flags Out:   N, Z, C
  uint8_t Executioner::ASL()
  {
    fetch();

    if (getAddressModeName() == "ACC")
    {
      cpu->incrementCycleCount();
    }
    else
    {
      cpu->writeMemory(addr_abs, fetched & 0x00FF);
    }

    uint16_t value = (uint16_t)fetched << 1;
    cpu->SetFlag(cpu->C, (value & 0xFF00) > 0);
    cpu->SetFlag(cpu->Z, (value & 0x00FF) == 0x00);
    cpu->SetFlag(cpu->N, value & 0x80);

    if (getAddressModeName() == "ACC")
    {
      //a = (uint8_t) (value & 0x00FF);
      cpu->setRegister(cpu->AC, (uint8_t)(value & 0x00FF));
    }
    else
    {
      cpu->writeMemory(addr_abs, (uint8_t)(value & 0x00FF));
    }

    if (getAddressModeName() == "ABX")
    {
      cpu->incrementCycleCount();
    }
    return 0;
  }


  // Instruction: Branch if Carry Clear
  // Function:    if(C == 0) pc = address 
  uint8_t Executioner::BCC()
  {
    branchOperation(cpu->GetFlag(cpu->C) == 0);
    return 0;
  }


  // Instruction: Branch if Carry Set
  // Function:    if(C == 1) pc = address
  uint8_t Executioner::BCS()
  {
    branchOperation(cpu->GetFlag(cpu->C) == 1);
    return 0;
  }


  // Instruction: Branch if Equal
  // Function:    if(Z == 1) pc = address
  uint8_t Executioner::BEQ()
  {
    branchOperation(cpu->GetFlag(cpu->Z) == 1);
    return 0;
  }


  // Instruction: Test Bits in Memory with Accumulator
  // Function:    A & M, M7 -> N, M6 -> V
  // Flags Out:   N, Z, V
  uint8_t Executioner::BIT()
  {
    fetch();
    //uint8_t value = a & fetched;
    uint8_t value = (cpu->getRegister(cpu->AC) & fetched);

    cpu->SetFlag(cpu->Z, (value & 0xFF) == 0x00);
    cpu->SetFlag(cpu->N, fetched & (1 << 7));
    cpu->SetFlag(cpu->V, fetched & (1 << 6));
    return 0;
  }


  // Instruction: Branch if Negative
  // Function:    if(N == 1) pc = address
  uint8_t Executioner::BMI()
  {
    branchOperation(cpu->GetFlag(cpu->N) == 1);
    return 0;
  }


  // Instruction: Branch if Not Equal
  // Function:    if(Z == 0) pc = address
  uint8_t Executioner::BNE()
  {
    branchOperation(cpu->GetFlag(cpu->Z) == 0);
    return 0;
  }


  // Instruction: Branch if Positive
  // Function:    if(N == 0) pc = address
  uint8_t Executioner::BPL()
  {
    branchOperation(cpu->GetFlag(cpu->N) == 0);
    return 0;
  }


  // Instruction: Break
  // Function:    Program Sourced Interrupt
  uint8_t Executioner::BRK()
  {
    breakOperation(true, 0xFFFE);
    return 0;
  }


  // Instruction: Branch if Overflow Clear
  // Function:    if(V == 0) pc = address
  uint8_t Executioner::BVC()
  {
    branchOperation(cpu->GetFlag(cpu->V) == 0);
    return 0;
  }


  // Instruction: Branch if Overflow Set
  // Function:    if(V == 1) pc = address
  uint8_t Executioner::BVS()
  {
    branchOperation(cpu->GetFlag(cpu->V) == 1);
    return 0;
  }


  // Instruction: Clear Carry Flag
  // Function:    C = 0
  uint8_t Executioner::CLC()
  {
    cpu->SetFlag(cpu->C, false);
    cpu->incrementCycleCount();
    return 0;
  }


  // Instruction: Clear Decimal Flag
  // Function:    D = 0
  uint8_t Executioner::CLD()
  {
    cpu->SetFlag(cpu->D, false);
    cpu->incrementCycleCount();
    return 0;
  }


  // Instruction: Disable Interrupts / Clear Interrupt Flag
  // Function:    I = 0
  uint8_t Executioner::CLI()
  {
    cpu->SetFlag(cpu->I, false);
    cpu->incrementCycleCount();
    return 0;
  }


  // Instruction: Clear Overflow Flag
  // Function:    V = 0
  uint8_t Executioner::CLV()
  {
    cpu->SetFlag(cpu->V, false);
    cpu->incrementCycleCount();
    return 0;
  }

  // Instruction: Compare Accumulator
  // Function:    C <- A >= M      Z <- (A - M) == 0
  // Flags Out:   N, C, Z
  uint8_t Executioner::CMP()
  {
    fetch();
    //uint8_t value = (uint16_t)a - (uint16_t)fetched;
    uint8_t value = (cpu->getRegister(cpu->AC) - fetched);
    cpu->SetFlag(cpu->C, cpu->getRegister(cpu->AC) >= fetched);
    cpu->SetFlag(cpu->Z, (value & 0x00FF) == 0x0000);
    cpu->SetFlag(cpu->N, value & 0x0080);
    return 1;
  }


  // Instruction: Compare X Register
  // Function:    C <- X >= M      Z <- (X - M) == 0
  // Flags Out:   N, C, Z
  uint8_t Executioner::CPX()
  {
    fetch();
    //uint8_t value = (uint16_t)x - (uint16_t)fetched;
    uint8_t value = (cpu->getRegister(cpu->X) - fetched);
    cpu->SetFlag(cpu->C, cpu->getRegister(cpu->X) >= fetched);
    cpu->SetFlag(cpu->Z, (value & 0x00FF) == 0x0000);
    cpu->SetFlag(cpu->N, value & 0x0080);
    return 0;
  }


  // Instruction: Compare Y Register
  // Function:    C <- Y >= M      Z <- (Y - M) == 0
  // Flags Out:   N, C, Z
  uint8_t Executioner::CPY()
  {
    fetch();
    //uint16_t value = (uint16_t)y - (uint16_t)fetched;
    uint8_t value = (cpu->getRegister(cpu->Y) - fetched);
    cpu->SetFlag(cpu->C, cpu->getRegister(cpu->Y) >= fetched);
    cpu->SetFlag(cpu->Z, (value & 0x00FF) == 0x0000);
    cpu->SetFlag(cpu->N, value & 0x0080);
    return 0;
  }


  // Instruction: Decrement Value at Memory Location
  // Function:    M = M - 1
  // Flags Out:   N, Z
  uint8_t Executioner::DEC()
  {
    fetch();

    cpu->writeMemory(addr_abs, fetched & 0x00FF);
    temp = fetched - 1;
    cpu->SetFlag(cpu->Z, (temp & 0x00FF) == 0x0000);
    cpu->SetFlag(cpu->N, temp & 0x0080);

    cpu->writeMemory(addr_abs, temp & 0x00FF);

    if (getAddressModeName() == "ABX")
    {
      cpu->incrementCycleCount();
    }

    return 0;
  }


  // Instruction: Decrement X Register
  // Function:    X = X - 1
  // Flags Out:   N, Z
  uint8_t Executioner::DEX()
  {
    //temp = x - 1;
    //x = temp & 0x00FF;
    //cpu->SetFlag(cpu->Z, (temp & 0x00FF) == 0x0000);
    //cpu->SetFlag(cpu->N, temp & 0x0080);

    uint8_t value = cpu->getRegister(cpu->X);
    --value;
    cpu->setRegister(cpu->X, value);
    cpu->SetFlag(cpu->Z, (value & 0xFF) == 0x00);
    cpu->SetFlag(cpu->N, value & 0x80);

    cpu->incrementCycleCount();
    return 0;
  }


  // Instruction: Decrement Y Register
  // Function:    Y = Y - 1
  // Flags Out:   N, Z
  uint8_t Executioner::DEY()
  {
    //temp = y - 1;
    //y = temp & 0x00FF;
    //cpu->SetFlag(cpu->Z, (temp & 0x00FF) == 0x0000);
    //cpu->SetFlag(cpu->N, temp & 0x0080);

    uint8_t value = cpu->getRegister(cpu->Y);
    --value;
    cpu->setRegister(cpu->Y, value);
    cpu->SetFlag(cpu->Z, (value & 0xFF) == 0x00);
    cpu->SetFlag(cpu->N, value & 0x80);

    cpu->incrementCycleCount();
    return 0;
  }


  // Instruction: Bitwise Logic XOR
  // Function:    A = A xor M
  // Flags Out:   N, Z
  uint8_t Executioner::EOR()
  {
    fetch();
    //a = a ^ fetched;
    //cpu->SetFlag(cpu->Z, a == 0x00);
    //cpu->SetFlag(cpu->N, a & 0x80);

    uint8_t value = cpu->getRegister(cpu->AC) ^ fetched;
    cpu->setRegister(cpu->AC, value);
    cpu->SetFlag(cpu->Z, (value & 0xFF) == 0x00);
    cpu->SetFlag(cpu->N, value & 0x80);

    return 1;
  }


  // Instruction: Increment Value at Memory Location
  // Function:    M = M + 1
  // Flags Out:   N, Z
  uint8_t Executioner::INC()
  {
    fetch();

    cpu->writeMemory(addr_abs, fetched & 0x00FF);
    temp = fetched + 1;

    cpu->SetFlag(cpu->Z, (temp & 0x00FF) == 0x0000);
    cpu->SetFlag(cpu->N, temp & 0x0080);

    cpu->writeMemory(addr_abs, temp & 0x00FF);

    if (getAddressModeName() == "ABX")
    {
      cpu->incrementCycleCount();
    }

    return 0;
  }


  // Instruction: Increment X Register
  // Function:    X = X + 1
  // Flags Out:   N, Z
  uint8_t Executioner::INX()
  {
    //temp = x + 1;
    //x = temp & 0x00FF;
    //cpu->SetFlag(cpu->Z, (temp & 0x00FF) == 0x0000);
    //cpu->SetFlag(cpu->N, temp & 0x0080);

    uint8_t value = cpu->getRegister(cpu->X);
    ++value;
    cpu->setRegister(cpu->X, value);
    cpu->SetFlag(cpu->Z, (value & 0xFF) == 0x00);
    cpu->SetFlag(cpu->N, value & 0x80);

    cpu->incrementCycleCount();
    return 0;
  }


  // Instruction: Increment Y Register
  // Function:    Y = Y + 1
  // Flags Out:   N, Z
  uint8_t Executioner::INY()
  {
    //temp = y + 1;
    //y = temp & 0x00FF;
    //cpu->SetFlag(cpu->Z, (temp & 0x00FF) == 0x0000);
    //cpu->SetFlag(cpu->N, temp & 0x0080);

    uint8_t value = cpu->getRegister(cpu->Y);
    ++value;
    cpu->setRegister(cpu->Y, value);
    cpu->SetFlag(cpu->Z, (value & 0xFF) == 0x00);
    cpu->SetFlag(cpu->N, value & 0x80);

    cpu->incrementCycleCount();
    return 0;
  }


  // Instruction: Jump To Location
  // Function:    pc = address
  uint8_t Executioner::JMP()
  {
    //pc = addr_abs;
    cpu->setProgramCounter(addr_abs);
    return 0;
  }


  // Instruction: Jump To Sub-Routine
  // Function:    Push current pc to stack, pc = address
  uint8_t Executioner::JSR()
  {
    cpu->incrementCycleCount();
    cpu->decrementProgramCounter();

    uint16_t pc = cpu->getProgramCounter();

    cpu->PokeStack((pc >> 8) & 0x00FF);
    cpu->incrementCycleCount();
    cpu->decrementStackPointer();

    cpu->PushStack(pc & 0x00FF);

    cpu->setProgramCounter(addr_abs);
    return 0;
  }


  // Instruction: Load The Accumulator
  // Function:    A = M
  // Flags Out:   N, Z
  uint8_t Executioner::LDA()
  {
    fetch();

    //a = fetched;
    //cpu->SetFlag(cpu->Z, a == 0x00);
    //cpu->SetFlag(cpu->N, a & 0x80);

    cpu->setRegister(cpu->AC, fetched);
    cpu->SetFlag(cpu->Z, fetched == 0x00);
    cpu->SetFlag(cpu->N, fetched & 0x80);
    return 1;
  }


  // Instruction: Load The X Register
  // Function:    X = M
  // Flags Out:   N, Z
  uint8_t Executioner::LDX()
  {
    fetch();
    //x = fetched;
    //cpu->SetFlag(cpu->Z, x == 0x00);
    //cpu->SetFlag(cpu->N, x & 0x80);

    cpu->setRegister(cpu->X, fetched);
    cpu->SetFlag(cpu->Z, fetched == 0x00);
    cpu->SetFlag(cpu->N, fetched & 0x80);
    return 1;
  }


  // Instruction: Load The Y Register
  // Function:    Y = M
  // Flags Out:   N, Z
  uint8_t Executioner::LDY()
  {
    fetch();

    //y = fetched;
    //cpu->SetFlag(cpu->Z, y == 0x00);
    //cpu->SetFlag(cpu->N, y & 0x80);

    cpu->setRegister(cpu->Y, fetched);
    cpu->SetFlag(cpu->Z, fetched == 0x00);
    cpu->SetFlag(cpu->N, fetched & 0x80);
    return 1;
  }


  // Instruction: Logical Shift Right
  // Function:    A = C <- (A << 1) <- 0
  // Flags Out:   N=0, Z, C
  uint8_t Executioner::LSR()
  {
    fetch();

    if (getAddressModeName() == "ACC")
    {
      cpu->incrementCycleCount();
    }
    else
    {
      cpu->writeMemory(addr_abs, fetched & 0x00FF);
    }

    cpu->SetFlag(cpu->C, fetched & 0x0001);
    temp = fetched >> 1;
    cpu->SetFlag(cpu->Z, (temp & 0x00FF) == 0x0000);
    cpu->SetFlag(cpu->N, temp & 0x0080);


    uint8_t value = temp & 0x00FF;

    if (getAddressModeName() == "ACC")
    {
      cpu->setRegister(cpu->AC, value);
    }
    else
    {
      cpu->writeMemory(addr_abs, value);
    }

    if (getAddressModeName() == "ABX")
    {
      cpu->incrementCycleCount();
    }
    return 0;
  }


  // Instruction: No Operation
  // Function:    -
  uint8_t Executioner::NOP()
  {
    cpu->incrementCycleCount();
    return 0;
  }


  // Instruction: Bitwise Logic OR
  // Function:    A = A | M
  // Flags Out:   N, Z
  uint8_t Executioner::ORA()
  {
    fetch();
    //a = a | fetched;
    //cpu->SetFlag(cpu->Z, a == 0x00);
    //cpu->SetFlag(cpu->N, a & 0x80);

    uint8_t value = cpu->getRegister(cpu->AC) | fetched;
    cpu->setRegister(cpu->AC, value);
    cpu->SetFlag(cpu->Z, value == 0x00);
    cpu->SetFlag(cpu->N, value & 0x80);
    return 1;
  }


  // Instruction: Push Accumulator to Stack
  // Function:    A -> stack
  uint8_t Executioner::PHA()
  {
    //cpu->PushStack(a);
    uint8_t ac = cpu->getRegister(cpu->AC);
    cpu->PushStack(ac);
    cpu->incrementCycleCount();
    Logger::log()->debug("OP {} - newSR: {} {: >56}", getOperation(), cpu->DecodeFlag(ac), cpu->reg);
    return 0;
  }


  // Instruction: Push Status Register to Stack
  // Function:    status -> stack
  // Note:        Break flag is set to 1 before push
  uint8_t Executioner::PHP()
  {
    uint8_t status = cpu->getRegister(cpu->SR) | cpu->B | cpu->U;

#ifdef DEBUG
    Logger::log()->debug("OP {} - GOT SR: {} {: >64}", getOperation(), cpu->DecodeFlag(status), cpu->getRegister(cpu->SR));
#endif
    cpu->PushStack(status);
    cpu->incrementCycleCount();
#ifdef DEBUG
    cpu->DumpStackAtPointer();
#endif
    return 0;
  }


  // Instruction: Pop Accumulator off Stack
  // Function:    A <- stack
  // Flags Out:   N, Z
  uint8_t Executioner::PLA()
  {
    //stkp++;
    //cpu->incrementCycleCount();
    //a = cpu->PopStack();
    uint8_t value = cpu->PopStack();
    cpu->incrementCycleCount();
    cpu->setRegister(cpu->AC, value);
    cpu->SetFlag(cpu->Z, value == 0x00);
    cpu->SetFlag(cpu->N, value & 0x80);
    cpu->incrementCycleCount();
    return 0;
  }


  // Instruction: Pop Status Register off Stack
  // Function:    Status <- stack
  uint8_t Executioner::PLP()
  {
    //stkp++;
    //cpu->incrementStackPointer();
    //cpu->incrementCycleCount();
    //status = cpu->PeekStack();

    //cpu->setRegister(cpu->SR, cpu->PeekStack());
    cpu->setRegister(cpu->SR, cpu->PopStack());
    cpu->incrementCycleCount();

    //cpu->incrementCycleCount();
    cpu->SetFlag(cpu->U, 1);
    cpu->incrementCycleCount();

    return 0;
  }


  // Instruction: Rotate Left
  // Function:    (C << 1)
  // Flags Out:    N, Z, C
  uint8_t Executioner::ROL()
  {
    fetch();

    if (getAddressModeName() == "ACC")
    {
      cpu->incrementCycleCount();
    }
    else
    {
      cpu->writeMemory(addr_abs, fetched & 0x00FF);
    }

    temp = (uint16_t)(fetched << 1) | cpu->GetFlag(cpu->C);
    cpu->SetFlag(cpu->C, temp & 0xFF00);
    cpu->SetFlag(cpu->Z, (temp & 0x00FF) == 0x0000);
    cpu->SetFlag(cpu->N, temp & 0x0080);

    if (getAddressModeName() == "ACC")
    {
      cpu->setRegister(cpu->AC, (uint8_t)(temp & 0x00FF));
    }
    else
    {
      cpu->writeMemory(addr_abs, temp & 0x00FF);
    }

    if (getAddressModeName() == "ABX")
    {
      cpu->incrementCycleCount();
    }

    return 0;
  }


  // Instruction: Rotate Right
  // Function:    (C >> 1)
  // Flags Out:    N, Z, C
  uint8_t Executioner::ROR()
  {
    fetch();

    if (getAddressModeName() == "ACC")
    {
      cpu->incrementCycleCount();
    }
    else
    {
      cpu->writeMemory(addr_abs, fetched & 0x00FF);
    }

    temp = (uint16_t)(cpu->GetFlag(cpu->C) << 7) | (fetched >> 1);
    cpu->SetFlag(cpu->C, fetched & 0x01);
    cpu->SetFlag(cpu->Z, (temp & 0x00FF) == 0x00);
    cpu->SetFlag(cpu->N, temp & 0x0080);

    if (getAddressModeName() == "ACC")
    {
      cpu->setRegister(cpu->AC, (uint8_t)(temp & 0x00FF));
    }
    else
    {
      cpu->writeMemory(addr_abs, temp & 0x00FF);
    }

    if (getAddressModeName() == "ABX")
    {
      cpu->incrementCycleCount();
    }

    return 0;
  }


  // Instruction: Return from Interrupt
  // Function:    Pull SR, Pull PC
  // Flags Out:    From Stack
  uint8_t Executioner::RTI()
  {
#ifdef DEBUG
    cpu->DumpStack();
    Logger::log()->debug("OP {} - {: >50}", getOperation(), cpu->reg);
#endif

    cpu->incrementCycleCount();

    cpu->incrementStackPointer();
    cpu->incrementCycleCount();

#ifdef DEBUG
    cpu->DumpStackAtPointer();
    Logger::log()->debug("OP {} - {: >50}", getOperation(), cpu->reg);
#endif

    uint8_t newSR = cpu->PeekStack();
#ifdef DEBUG
    cpu->DumpStackAtPointer();
    Logger::log()->debug("OP {} - newSR: 0x{:02X} {} {: >50}", getOperation(), newSR, cpu->DecodeFlag(newSR), cpu->reg);
#endif
    //cpu->incrementStackPointer();
    cpu->incrementCycleCount();

    cpu->setRegister(cpu->SR, newSR & ~cpu->B & ~cpu->U);

#ifdef DEBUG
    cpu->DumpStackAtPointer();
    Logger::log()->debug("OP {} - SR SET {: >74}", getOperation(), cpu->getRegister(cpu->SR));
#endif

    uint16_t pcLo = cpu->PopStack();

    cpu->incrementStackPointer();
    uint16_t pcHi = cpu->PeekStack() << 8;
    
    uint16_t newPc = (pcHi | pcLo);
    cpu->incrementCycleCount();

#ifdef DEBUG
    Logger::log()->info("RTI - Old PC {:04X} - New PC {:04X}", cpu->getProgramCounter(), newPc);
#endif

    cpu->setProgramCounter(newPc);

#ifdef DEBUG
    cpu->DumpStackAtPointer();
    Logger::log()->debug("OP {} - SR SET {: >74}", getOperation(), cpu->getRegister(cpu->SR));
#endif

    return 0;
  }


  // Instruction: Return from Subroutine
  // Function:    Pull PC, PC+1 -> PC
  // Flags Out:    -
  uint8_t Executioner::RTS()
  {
    cpu->incrementCycleCount();
    uint16_t newPc = cpu->PopStack() | (cpu->PopStack() << 8);

    cpu->incrementCycleCount();
    cpu->setProgramCounter(newPc);


    cpu->incrementProgramCounter();
    cpu->incrementCycleCount();
    return 0;
  }


  // Instruction: Set Carry Flag
  // Function:    C = 1
  uint8_t Executioner::SEC()
  {
    cpu->SetFlag(cpu->C, true);
    cpu->incrementCycleCount();
    return 0;
  }


  // Instruction: Set Decimal Flag
  // Function:    D = 1
  uint8_t Executioner::SED()
  {
    cpu->SetFlag(cpu->D, true);
    cpu->incrementCycleCount();
    return 0;
  }


  // Instruction: Set Interrupt Flag / Enable Interrupts
  // Function:    I = 1
  uint8_t Executioner::SEI()
  {
    cpu->SetFlag(cpu->I, true);
    cpu->incrementCycleCount();
    return 0;
  }


  // Instruction: Store Accumulator at Address
  // Function:    M = A
  uint8_t Executioner::STA()
  {
    cpu->writeMemory(addr_abs, cpu->getRegister(cpu->AC));

    std::vector<std::string> affectedAddrModes;
    affectedAddrModes.push_back("ABX");
    affectedAddrModes.push_back("ABY");
    affectedAddrModes.push_back("IZY");

    if (in_array<std::string>(getAddressModeName(), affectedAddrModes))
    {
      cpu->incrementCycleCount();
    }

    return 0;
  }


  // Instruction: Store X Register at Address
  // Function:    M = X
  uint8_t Executioner::STX()
  {
    cpu->writeMemory(addr_abs, cpu->getRegister(cpu->X));
    return 0;
  }


  // Instruction: Store Y Register at Address
  // Function:    M = Y
  uint8_t Executioner::STY()
  {
    cpu->writeMemory(addr_abs, cpu->getRegister(cpu->Y));
    return 0;
  }


  // Instruction: Transfer Accumulator to X Register
  // Function:    X = A
  // Flags Out:   N, Z
  uint8_t Executioner::TAX()
  {
    cpu->incrementCycleCount();

    uint8_t value = cpu->getRegister(cpu->AC);
    cpu->setRegister(cpu->X, value);

    cpu->SetFlag(cpu->Z, value == 0x00);
    cpu->SetFlag(cpu->N, value & 0x80);
    return 0;
  }


  // Instruction: Transfer Accumulator to Y Register
  // Function:    Y = A
  // Flags Out:   N, Z
  uint8_t Executioner::TAY()
  {
    cpu->incrementCycleCount();

    uint8_t value = cpu->getRegister(cpu->AC);
    cpu->setRegister(cpu->Y, value);

    cpu->SetFlag(cpu->Z, value == 0x00);
    cpu->SetFlag(cpu->N, value & 0x80);
    return 0;
  }


  // Instruction: Transfer Stack Pointer to X Register
  // Function:    X = stack pointer
  // Flags Out:   N, Z
  uint8_t Executioner::TSX()
  {
    uint8_t value = cpu->getRegister(cpu->SP);
    cpu->setRegister(cpu->X, value);

    cpu->SetFlag(cpu->Z, value == 0x00);
    cpu->SetFlag(cpu->N, value & 0x80);
    cpu->incrementCycleCount();
    return 0;
  }


  // Instruction: Transfer X Register to Accumulator
  // Function:    A = X
  // Flags Out:   N, Z
  uint8_t Executioner::TXA()
  {
    cpu->incrementCycleCount();
    uint8_t value = cpu->getRegister(cpu->X);
    cpu->setRegister(cpu->AC, value);

    cpu->SetFlag(cpu->Z, value == 0x00);
    cpu->SetFlag(cpu->N, value & 0x80);
    return 0;
  }


  // Instruction: Transfer X Register to Stack Pointer
  // Function:    stack pointer = X
  uint8_t Executioner::TXS()
  {

    uint8_t value = cpu->getRegister(cpu->X);
    cpu->setRegister(cpu->SP, value);
    cpu->incrementCycleCount();
    return 0;
  }


  // Instruction: Transfer Y Register to Accumulator
  // Function:    A = Y
  // Flags Out:   N, Z
  uint8_t Executioner::TYA()
  {
    cpu->incrementCycleCount();

    uint8_t value = cpu->getRegister(cpu->Y);
    cpu->setRegister(cpu->AC, value);
    cpu->SetFlag(cpu->Z, value == 0x00);
    cpu->SetFlag(cpu->N, value & 0x80);
    return 0;
  }

#ifdef ILLEGAL
  // Illegal opcodes

  // Instruction: AND oper + LSR
  // Function:    A AND oper, 0 -> [76543210] -> C
  // Flags Out:   N, Z, C
  // Note:        AKA ASR
  uint8_t Executioner::ALR()
  {
    fetch();

    temp = cpu->getRegister(cpu->AC) & fetched;
    cpu->setRegister(cpu->AC, (uint8_t)(temp >> 1));

    cpu->SetFlag(cpu->Z, temp == 0x00);
    cpu->SetFlag(cpu->N, temp & 0x80);
    cpu->SetFlag(cpu->C, temp & 0x0001);
    return 0;
  }


  // Instruction: AND oper + set C as ASL
  // Function:    A AND oper, bit(7) -> C
  // Flags Out:   N, Z, C
  // OpCode:      0x0B
  uint8_t Executioner::ANC()
  {
    fetch();

    uint8_t value = cpu->getRegister(cpu->AC) & fetched;
    cpu->setRegister(cpu->AC, value);

    cpu->SetFlag(cpu->Z, value == 0x00);
    cpu->SetFlag(cpu->N, value & 0x80);
    cpu->SetFlag(cpu->C, (value & 0xFF00) > 0);
    return 0;
  }

  // Instruction: AND oper + set C as ROL
  // Function:    A AND oper, bit(7) -> C
  // Flags Out:   N, Z, C
  // OpCode:      0x2B
  // @see OPCode::ANC
  uint8_t Executioner::ANC2()
  {
    fetch();

    uint8_t value = cpu->getRegister(cpu->AC) & fetched;
    cpu->setRegister(cpu->AC, value);

    cpu->SetFlag(cpu->Z, value == 0x00);
    cpu->SetFlag(cpu->N, value & 0x80);
    cpu->SetFlag(cpu->C, value & 0xFF00);
    return 0;
  }


  // Instruction: * AND X + AND oper
  // Function:    (A OR CONST) AND X AND oper -> A
  // Flags Out:   N, Z
  // Note:        Highly unstable, involves a "magic" constant
  //              A base value in A is determined based on the
  //              contets of A and a constant, which may be
  //              typically $00, $ff, $ee, etc. The value of
  //              this constant depends on temerature, the chip
  //              series, and maybe other factors, as well.
  //              In order to eliminate these uncertaincies from
  //              the equation, use either 0 as the operand or a
  //              value of $FF in the accumulator.
  uint8_t Executioner::ANE()
  {
    fetch();

    uint8_t ac_value = cpu->getRegister(cpu->AC);
    uint8_t x_value = cpu->getRegister(cpu->X);


    ac_value = (ac_value ^ magic) & x_value & fetched;

    cpu->setRegister(cpu->AC, ac_value);

    cpu->SetFlag(cpu->Z, (ac_value & 0x00FF) == 0x00);
    cpu->SetFlag(cpu->N, ac_value & 0x0080);

    return 0;
  }


  // Instruction: AND oper + ROR
  // Function:    A AND oper, C -> [76543210] -> C
  // Flags Out:   N, Z, C, V
  uint8_t Executioner::ARR()
  {
    fetch();

    uint8_t value = cpu->getRegister(cpu->AC);
    temp = (cpu->GetFlag(cpu->C) << 7) | ((value & fetched) >> 1);
    cpu->SetFlag(cpu->C, fetched & 0x01);
    cpu->SetFlag(cpu->Z, (temp & 0x00FF) == 0x00);
    cpu->SetFlag(cpu->N, temp & 0x0080);
    cpu->SetFlag(cpu->V, (temp & 0x40) ^ ((temp & 0x20) << 1));

    if (getAddressModeName() == "IMP")
    {
      cpu->setRegister(cpu->AC, (uint8_t)(temp & 0x00FF));
    }
    else
    {
      cpu->writeMemory(addr_abs, temp & 0x00FF);
    }
    return 0;
  }


  // Instruction: DEC oper + CMP oper
  // Function:    M - 1 -> M, A - M
  // Flags Out:   N, Z, C
  uint8_t Executioner::DCP()
  {
    fetch();
    uint8_t value = cpu->getRegister(cpu->AC);
    temp = fetched - 1;
    //cpu->writeMemory(addr_abs, temp);
    cpu->writeMemory(addr_abs, temp & 0x00FF);
    cpu->SetFlag(cpu->C, value >= fetched);
    cpu->SetFlag(cpu->Z, (temp & 0x00FF) == 0x00);
    cpu->SetFlag(cpu->N, temp & 0x0080);
    return 0;
  }


  // Instruction: INC oper + SBC oper
  // Function:    M + 1 -> M, A - M - (C - 1) -> A
  // Flags Out:   N, Z, C, V
  uint8_t Executioner::ISC()
  {
    fetch();
    temp = fetched + 1;
    //cpu->writeMemory(addr_abs, temp);
    cpu->writeMemory(addr_abs, temp & 0x00FF);
    SBC();
    return 0;
  }


  // Instruction: LDA/TSX oper
  // Function:    M AND SP -> A, X, SP
  // Flags Out:   N, Z
  uint8_t Executioner::LAS()
  {
    fetch();
    uint8_t value = cpu->getRegister(cpu->SP);
    uint8_t result = value & fetched;
    cpu->setRegister(cpu->SP, result);
    cpu->setRegister(cpu->AC, result);
    cpu->setRegister(cpu->X, result);

    cpu->SetFlag(cpu->Z, (result & 0xFF) == 0x00);
    cpu->SetFlag(cpu->N, result & 0x80);

    return 1;
  }


  // Instruction: LDA oper + LDX oper
  // Function:    M -> A -> X
  // Flags Out:   N, Z
  uint8_t Executioner::LAX()
  {
    fetch();

    cpu->setRegister(cpu->AC, fetched);
    cpu->setRegister(cpu->X, fetched);

    cpu->SetFlag(cpu->Z, (fetched & 0xFF) == 0x00);
    cpu->SetFlag(cpu->N, fetched & 0x80);

    return 1;
  }


  // Instruction: Store * AND oper in A and X
  // Function:    (A OR CONST) AND oper -> A -> X
  // Flags Out:   N, Z
  // Note:        Highly unstable, involves a "magic" constant
  // See:         Processor::ANE
  uint8_t Executioner::LXA()
  {
    fetch();

    uint8_t value = (cpu->getRegister(cpu->AC) ^ magic) & fetched;

    cpu->setRegister(cpu->AC, value);
    cpu->setRegister(cpu->X, value);

    cpu->SetFlag(cpu->Z, (value & 0x00FF) == 0x00);
    cpu->SetFlag(cpu->N, value & 0x0080);

    return 0;
  }


  // Instruction: ROL oper + AND oper
  // Function:    M = C <- [76543210] <- C, A AND M -> A
  // Flags Out:   N, Z, C
  uint8_t Executioner::RLA()
  {
    fetch();
    temp = (uint16_t)((fetched << 1) & 0xFF) | cpu->GetFlag(cpu->C);
    cpu->writeMemory(addr_abs, temp & 0x00FF);
    cpu->SetFlag(cpu->C, (fetched & 0xFF00) > 0);
  Processor:AND();
    return 0;
  }


  // Instruction: ROL oper + ADC oper
  // Function:    M = C -> [76543210] -> C, A + M + C -> A, C
  // Flags Out:   N, Z, C, V
  uint8_t Executioner::RRA()
  {
    fetch();
    temp = (uint16_t)(fetched << 1) | cpu->GetFlag(cpu->C);
    cpu->writeMemory(addr_abs, temp & 0x00FF);
    cpu->SetFlag(cpu->C, (fetched & 0xFF00) > 0);
  Processor:ADC();
    return 0;
  }


  // Instruction: A and X are put on the bus at the same
  //              time (resulting effectively in an AND
  //              operation) and stored in M
  // Function:    A AND X -> M
  // Flags Out:   -
  uint8_t Executioner::SAX()
  {
    fetch();

    uint8_t value = cpu->getRegister(cpu->AC) & cpu->getRegister(cpu->X);
    cpu->writeMemory(addr_abs, value & 0x00FF);
    cpu->SetFlag(cpu->Z, value == 0x00);
    cpu->SetFlag(cpu->N, value & 0x80);
    return 0;
  }


  // Instruction: CMP and DEX at once, sets flags like CMP
  // Function:    (A AND X) - oper -> X
  // Flags Out:   N, Z, C
  uint8_t Executioner::SBX()
  {
    fetch();

    uint8_t value = (cpu->getRegister(cpu->AC) & cpu->getRegister(cpu->X)) - fetched;
    cpu->setRegister(cpu->X, value);
    //x = ((uint16_t)a & (uint16_t)x) - (uint16_t)fetched;
    cpu->SetFlag(cpu->C, value & 0xFF00);
    cpu->SetFlag(cpu->Z, (value & 0x00FF) == 0x0000);
    cpu->SetFlag(cpu->N, value & 0x0080);

    return 0;
  }


  // Instruction: Stores A AND X AND (high-byte of addr. + 1) at addr.
  // Function:    A AND X AND (H+1) -> M
  // Flags Out:   -
  // Note:        Unstable: Sometimes 'AND (H+1)' is dropped, page boundary
  //              crossings may not work (with the high-byte of the value used
  //              as the high-byte of the address).
  uint8_t Executioner::SHA()
  {
    fetch();
    //temp = ((uint16_t)a & (uint16_t)x) & (uint16_t)((addr_abs >> 8) + 1);
    //cpu->writeMemory(addr_abs, temp & 0x00FF);

    uint16_t value = ((uint16_t)cpu->getRegister(cpu->AC) & (uint16_t)cpu->getRegister(cpu->X));
    value &= (uint16_t)((addr_abs >> 8) + 1);
    cpu->writeMemory(addr_abs, (uint8_t)(temp & 0x00FF));

    return 0;
  }


  // Instruction: Stores X AND (high-byte of addr. + 1) at addr.
  // Function:    X AND (H+1) -> M
  // Flags Out:   -
  uint8_t Executioner::SHX()
  {
    fetch();
    //temp = ((uint16_t)x) & (uint16_t)((addr_abs >> 8) + 1);
    //cpu->writeMemory(addr_abs, temp & 0x00FF);

    uint16_t value = ((uint16_t)cpu->getRegister(cpu->X) & (uint16_t)((addr_abs >> 8) + 1));
    cpu->writeMemory(addr_abs, (uint8_t)(temp & 0x00FF));

    return 0;
  }


  // Instruction: Stores Y AND (high-byte of addr. + 1) at addr.
  // Function:    Y AND (H+1) -> M
  // Flags Out:   -
  uint8_t Executioner::SHY()
  {
    fetch();
    //temp = ((uint16_t)y) & (uint16_t)((addr_abs >> 8) + 1);
    //cpu->writeMemory(addr_abs, temp & 0x00FF);

    uint16_t value = ((uint16_t)cpu->getRegister(cpu->Y) & (uint16_t)((addr_abs >> 8) + 1));
    cpu->writeMemory(addr_abs, (uint8_t)(temp & 0x00FF));
    return 0;
  }


  // Instruction: ASL oper + ORA oper
  // Function:    M = C <- [76543210] <- 0, A OR M -> A
  // Flags Out:   N, Z, C
  uint8_t Executioner::SLO()
  {
    fetch();
    temp = (uint16_t)fetched << 1;
    cpu->SetFlag(cpu->C, (temp & 0xFF00) > 0);
    cpu->writeMemory(addr_abs, temp & 0x00FF);

    //a = a | fetched;
    uint8_t value = cpu->getRegister(cpu->AC) | fetched;
    cpu->SetFlag(cpu->Z, (value & 0xFF) == 0x00);
    cpu->SetFlag(cpu->N, value & 0x80);
    return 0;
  }


  // Instruction: LSR oper + EOR oper
  // Function:    M = 0 -> [76543210] -> 0, A EOR M -> A
  // Flags Out:   -
  uint8_t Executioner::SRE()
  {
    fetch();

    cpu->SetFlag(cpu->C, fetched & 0x0001);
    temp = fetched >> 1;
    cpu->SetFlag(cpu->Z, (temp & 0xFF) == 0x0000);
    cpu->SetFlag(cpu->N, temp & 0x80);

    uint8_t value = (uint8_t)(temp & 0xFF);

    if (getAddressModeName() == "IMP")
    {
      cpu->setRegister(cpu->AC, value);
    }
    else
    {
      cpu->writeMemory(addr_abs, value);
    }

    cpu->setRegister(cpu->AC, value ^ fetched);

    return 0;
  }


  // Instruction: Puts A AND X in SP and stores A AND X AND (high-byte of addr. + 1) at addr.
  // Function:    A AND X -> SP, A AND X AND (H + 1) -> M
  // Flags Out:   -
  uint8_t Executioner::TAS()
  {
    fetch();
    uint8_t value = cpu->getRegister(cpu->AC) & cpu->getRegister(cpu->X);
    cpu->setRegister(cpu->SP, value);

    uint8_t h = (addr_abs >> 8);
    uint8_t h1 = cpu->readMemoryWithoutCycle(cpu->getProgramCounter() - 1);
    uint8_t r = (value & h1);

    if (cpu->extra_cycles > 0)
    {
      // We assume no DMA
      r &= h;
      uint16_t tasAddr = (r << 8) | (addr_abs & 0xFF);
      cpu->writeMemory(tasAddr, r);
    }
    else
    {
      cpu->writeMemory(addr_abs, (r & (h + 1)));
    }

    return 0;
  }


  // Alias for SBC
  uint8_t Executioner::USBC()
  {
    return SBC();
  }


  // Instruction: No Operation (Skip Byte)
  // Function:    -
  // Flags Out:   -
  // Note:        -
  uint8_t Executioner::DOP()
  {
    // Sadly not all NOPs are equal, Ive added a few here
    // based on https://wiki.nesdev.com/w/index.php/CPU_unofficial_opcodes
    // and will add more based on game compatibility, and ultimately
    // I'd like to cover all illegal opcodes too
    //cpu->incrementCycleCount();
    cpu->incrementCycleCount();
    return 0;
  }


  // Instruction: No Operation (Ignore)
  // Function:    -
  // Flags Out:   -
  // Note:        -
  uint8_t Executioner::TOP()
  {
    cpu->incrementCycleCount();
    return 0;
  }


  // This instruction freezes the CPU.
  // The processor will be trapped infinitely in
  // T1 phase with $FF on the data bus.
  // — Reset required.
  uint8_t Executioner::JAM()
  {
    cpu->setJammed();
    breakOperation(false, cpu->getProgramCounter());
    //throw std::exception();
    return 0;
  }
#else
  // This function captures illegal opcodes
  // Only needed when ILLEGAL macro is not set
  uint8_t Executioner::XXX()
  {
    return 0;
  }
#endif

#pragma endregion INSTRUCTION IMPLEMENTATIONS
};
