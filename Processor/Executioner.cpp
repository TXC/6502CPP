#include "Executioner.hpp"
#include "CPU.hpp"
#include "Logger.hpp"
#include "Types.hpp"
#include "Exceptions.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <map>
#include <stdexcept>

#include <spdlog/spdlog.h>
#if defined SPDLOG_FMT_EXTERNAL
#include <fmt/format.h>
#else
#include <spdlog/fmt/fmt.h>
#endif

namespace Processor
{
  Executioner::Executioner()
  {
    //loadInstructions();
    //opcode = &cpu->opcode;
    //cycle_count = &cpu->cycle_count;
    //operation_cycle = &cpu->operation_cycle;
  }

  Executioner::~Executioner()
  {
    // Destructor - has nothing to do
  }

  void Executioner::reset()
  {
    // get address to set program counter to
    addr_abs = 0xFFFC;

    uint16_t newPc = (cpu->readMemory(addr_abs + 1) << 8) | cpu->readMemory(addr_abs + 0);

    // set it
    cpu->setProgramCounter(newPc);

    // Clear internal helper variables
    addr_rel = 0x0000;
    addr_abs = 0x0000;
    fetched = 0x00;
  }

  void Executioner::execute()
  {
    execute(cpu->opcode);
  }

  void Executioner::execute(uint8_t op)
  {
    if (!lookup.contains(op)) {
      throw IllegalInstruction(fmt::format("Instruction 0x{:02X} could not be found", op));
      //throw std::runtime_error(fmt::format("Invalid operation ({:02X})", op));
    }

    try
    {
#if defined DEBUG
      Logger::log()->info("ADDR MODE START    - OP {} {: >53}", getOperation(), cpu->reg);
#endif

      (this->*lookup.at(op).addrmode.op)();

#if defined DEBUG
      Logger::log()->info("ADDR MODE FINISHED - OP {} {: >53}", getOperation(), cpu->reg);
#endif
    }
    catch (const std::exception& e)
    {
      std::cout << "std::exception : lookup.addrmode.op["
                << lookup.at(op).addrmode.name
                << "] reported an exception:"
                << e.what()
                << std::endl;
#if defined DEBUG
      print_exception(e);
#endif
    }
    catch (...)
    {
      std::cout << "catch_all : lookup.addrmode.op["
                << lookup.at(op).addrmode.name
                << "] reported an exception:"
                << std::endl;
    }

#if defined LOGMODE
    Logger::log()->info(
      "{:>10d}:{:02X} OP: 0x{:02X} / {}:{} {: <17s} {}",
      cpu->operation_cycle, cpu->cycle_count, op, getInstructionName(), getAddressModeName(),
      "XXX", cpu->reg
    );
#endif

    try
    {
#if defined DEBUG
      Logger::log()->info("OPERATION START    - OP {} {: >53}", getOperation(), cpu->reg);
#endif

      (this->*lookup.at(op).operate.op)();

#if defined DEBUG
      Logger::log()->info("OPERATION FINISHED - OP {} {: >53}", getOperation(), cpu->reg);
#endif

    }
    catch (const std::exception& e)
    {
      std::cout << "std::exception : lookup.operate.op["
                << lookup.at(op).operate.name
                << "] reported an exception:"
                << e.what()
                << std::endl;
#if defined DEBUG
      print_exception(e);
#endif
    }
    catch (...)
    {
      std::cout << "catch_all : lookup.operate.op["
                << lookup.at(op).addrmode.name
                << "] reported an exception:"
                << std::endl;
    }
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
    return lookup.at(op).addrmode.name;
  }

  std::string Executioner::getInstructionName()
  {
    return getInstructionName(cpu->opcode);
  }

  std::string Executioner::getInstructionName(uint8_t op)
  {
    if (lookup.contains(op) == false) {
      throw std::runtime_error(fmt::format("Unable to get Instruction Name - Invalid operation ({:02X})", op));
    }
    return lookup.at(op).operate.name;
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

    return fmt::format("{}:{} [{:02X}]", lookup.at(op).operate.name, lookup.at(op).addrmode.name, op);
  }

  void Executioner::printInstructions()
  {
    fmt::print("+      ");
    uint8_t i, j;
    for (i = 0x0; i <= 0xF; ++i)
    {
      fmt::print("+  -{:X}  ", i);
      if (i == 0xF)
      {
        fmt::print("+\n");
      }
    }

    for (i = 0x0; i <= 0xF; ++i)
    {
      fmt::print("+  {:X}-  ", i);
      for (j = 0x0; j <= 0xF; ++j)
      {
        if (lookup.contains((i << 4) | j)) {
          fmt::print("+ 0x{:02X} ", (i << 4) | j);
        }
        else
        {
          fmt::print("+      ");
        }
      }
      fmt::print("+\n");
    }
    //for (auto const& [key, val] : lookup)
    //{
    //  fmt::print("INSTRUCTION: {:02X} - {} - {} - {}\n", key, val.operate.name, val.addrmode.name, val.cycles);
    //}
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
#if defined DEBUG
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
  void Executioner::addrACC()
  {
    fetched = cpu->getRegisterAC();

#if defined DEBUG
    Logger::log()->debug(
      "OP {} {: >75}",
      getOperation(), cpu->reg
    );
#endif
  }

  // Address Mode: Implied
  // There is no additional data required for this instruction. The instruction
  // does something very simple like like sets a status bit. However, we will
  // target the accumulator, for instructions like PHA
  void Executioner::addrIMP()
  {
    fetched = 0x0;
  }

  // Address Mode: Immediate
  // The instruction expects the next byte to be used as a value, so we'll prep
  // the read address to point to the next byte
  void Executioner::addrIMM()
  {
    addr_abs = cpu->getProgramCounter();
    cpu->incrementProgramCounter();
#if defined DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} {: >57}",
      getOperation(), addr_abs, cpu->reg
    );
#endif
  }

  // Address Mode: Zero Page
  // To save program bytes, zero page addressing allows you to absolutely address
  // a location in first 0xFF bytes of address range. Clearly this only requires
  // one byte instead of the usual two.
  void Executioner::addrZP0()
  {
#if defined DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} {: >58}",
      getOperation(), addr_abs, cpu->reg
    );
#endif

    addr_abs = cpu->readMemory(cpu->getProgramCounter());

    cpu->incrementProgramCounter();
    addr_abs &= 0x00FF;

#if defined DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} {: >58}",
      getOperation(), addr_abs, cpu->reg
    );
#endif
  }

  // Address Mode: Zero Page with X Offset
  // Fundamentally the same as Zero Page addressing, but the contents of the X Register
  // is added to the supplied single byte address. This is useful for iterating through
  // ranges within the first page.
  void Executioner::addrZPX()
  {
#if defined DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} {: >57}",
      getOperation(), addr_abs, cpu->reg
    );
#endif

    addr_abs = cpu->readMemory(cpu->getProgramCounter());
    cpu->incrementCycleCount();

    addr_abs += cpu->getRegisterX();

    cpu->incrementProgramCounter();
    addr_abs &= 0x00FF;

#if defined DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} {: >57}",
      getOperation(), addr_abs, cpu->reg
    );
#endif
  }

  // Address Mode: Zero Page with Y Offset
  // Same as above but uses Y Register for offset
  void Executioner::addrZPY()
  {
#if defined DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} {: >57}",
      getOperation(), addr_abs, cpu->reg
    );
#endif
    addr_abs = cpu->readMemory(cpu->getProgramCounter());
    cpu->incrementCycleCount();

    addr_abs += cpu->getRegisterY();

    cpu->incrementProgramCounter();
    addr_abs &= 0x00FF;

#if defined DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} {: >57}",
      getOperation(), addr_abs, cpu->reg
    );
#endif
  }

  // Address Mode: Relative
  // This address mode is exclusive to branch instructions. The address
  // must reside within -128 to +127 of the branch instruction, i.e.
  // you cant directly branch to any address in the addressable range.
  void Executioner::addrREL()
  {
    addr_rel = cpu->readMemory(cpu->getProgramCounter());

#if defined DEBUG
    Logger::log()->debug(
      "OP {} - addr_rel: {:04X} {: >57}",
      getOperation(), addr_rel, cpu->reg
    );
#endif

    uint16_t a2 = (addr_rel + 1) & 0xFFFF;
#if defined DEBUG
    cpu->dumpRam(cpu->getProgramCounter());
#endif
    //cpu->incrementProgramCounter();
    if (addr_rel & 0x80)
    {
#if defined DEBUG
      Logger::log()->debug(
        "OP {} - addr_rel [PB]: {:04X} -> {:04X} {: >40}",
        getOperation(), addr_rel, addr_rel | 0xFF00, cpu->reg
      );
#endif
      //addr_rel = a2 - ((addr_rel ^ 0xFF) + 1);
      //addr_rel |= 0xFF00;
      addr_rel = (a2 | 0xFF00);
    }
    else
    {
      addr_rel = a2;
    }
#if defined DEBUG
    Logger::log()->debug(
      "OP {} - addr_rel: {:04X} {: >57}",
      getOperation(), addr_rel, cpu->reg
    );
#endif
  }

  // Address Mode: Absolute 
  // A full 16-bit address is loaded and used
  void Executioner::addrABS()
  {
#if defined DEBUG
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

#if defined DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} HI:{:02X} LO:{:02X} {: >45}",
      getOperation(), addr_abs, hi, lo, cpu->reg
    );
#endif
  }

  // Address Mode: Absolute with X Offset
  // Fundamentally the same as absolute addressing, but the contents of the X Register
  // is added to the supplied two byte address. If the resulting address changes
  // the page, an additional clock cycle is required
  void Executioner::addrABX()
  {
#if defined DEBUG
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
    addr_abs += cpu->getRegisterX();

#if defined DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} HI:{:02X} LO:{:02X} {: >45}",
      getOperation(), addr_abs, hi, lo, cpu->reg
    );
#endif

    std::vector<uint8_t> ignoredOpCodes;
    ignoredOpCodes.push_back(0x9D); // STA
    ignoredOpCodes.push_back(0xDE); // DEC
    ignoredOpCodes.push_back(0xFE); // INC

#ifndef EMULATE65C02
    ignoredOpCodes.push_back(0x1E); // ASL
    ignoredOpCodes.push_back(0x3E); // LSR
    ignoredOpCodes.push_back(0x5E); // ROL
    ignoredOpCodes.push_back(0x7E); // ROR
#endif

    if ((addr_abs & 0xFF00) != (hi << 8) && !in_array<uint8_t>(cpu->opcode, ignoredOpCodes))
    {
      cpu->addExtraCycle();
#if defined DEBUG
      Logger::log()->debug(
        "OP {} - addr_abs (Page Boundary): {:04X} HI:{:02X} LO:{:02X} {: >40}",
        getOperation(), addr_abs, hi, lo, cpu->reg
      );
#endif
    }
#if defined LOGMODE
    else if (in_array<uint8_t>(cpu->opcode, ignoredOpCodes))
    {
      Logger::log()->info("OP {} - Caught ignored OPCode", getOperation());
    }
#endif
  }

  // Address Mode: Absolute with Y Offset
  // Fundamentally the same as absolute addressing, but the contents of the Y Register
  // is added to the supplied two byte address. If the resulting address changes
  // the page, an additional clock cycle is required
  void Executioner::addrABY()
  {
#if defined DEBUG
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
    addr_abs += cpu->getRegisterY();

#if defined DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} HI:{:02X} LO:{:02X} {: >45}",
      getOperation(), addr_abs, hi, lo, cpu->reg
    );
#endif

    std::vector<uint8_t> ignoredOpCodes;
    ignoredOpCodes.push_back(0x99);
    if ((addr_abs & 0xFF00) != (hi << 8) && !in_array<uint8_t>(cpu->opcode, ignoredOpCodes))
    {
      cpu->addExtraCycle();
#if defined DEBUG
      Logger::log()->debug(
        "OP {} - addr_abs (Page Boundary): {:04X} HI:{:02X} LO:{:02X} {: >40}",
        getOperation(), addr_abs, hi, lo, cpu->reg
      );
#endif
    }
#if defined LOGMODE
    else if (in_array<uint8_t>(cpu->opcode, ignoredOpCodes))
    {
      Logger::log()->info("OP {} - Caught ignored OPCode", getOperation());
    }
#endif
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
  void Executioner::addrIND()
  {
#if defined DEBUG
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
#if defined DEBUG
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
#if defined DEBUG
      Logger::log()->debug(
        "OP {} addr_abs: {:04X} PTR:{:04X} HI:{:02X} LO:{:02X} {: >40}",
        getOperation(), addr_abs, ptr, ptr_hi, ptr_lo, cpu->reg
      );
#endif
#ifndef EMULATE65C02
    }
#endif
  }

  // Address Mode: Indirect X
  // The supplied 8-bit address is offset by X Register to index
  // a location in page 0x00. The actual 16-bit address is read 
  // from this location
  void Executioner::addrIZX()
  {
    uint16_t t = cpu->readMemory(cpu->getProgramCounter());

#if defined DEBUG
    Logger::log()->debug(
      "OP {} - t: 0x{:02X} {: >64}",
      getOperation(), t, cpu->reg
    );
#endif

    cpu->incrementProgramCounter();
    cpu->incrementCycleCount();

    uint8_t x = cpu->getRegisterX();
    uint16_t lo = cpu->readMemory((uint16_t)(t + (uint16_t)(size_t)x) & 0x00FF);
    uint16_t hi = cpu->readMemory((uint16_t)(t + (uint16_t)(size_t)x + 1) & 0x00FF);

    addr_abs = (hi << 8) | lo;

#if defined DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} HI:{:02X} LO:{:02X} {: >45}",
      getOperation(), addr_abs, hi, lo, cpu->reg
    );
#endif
  }

  // Address Mode: Indirect Y
  // The supplied 8-bit address indexes a location in page 0x00. From 
  // here the actual 16-bit address is read, and the contents of
  // Y Register is added to it to offset it. If the offset causes a
  // change in page then an additional clock cycle is required.
  void Executioner::addrIZY()
  {
    uint16_t t = cpu->readMemory(cpu->getProgramCounter());

#if defined DEBUG
    Logger::log()->debug(
      "OP {} - t: 0x{:02X} {: >64}",
      getOperation(), t, cpu->reg
    );
#endif

    cpu->incrementProgramCounter();

    uint16_t lo = cpu->readMemory(t & 0x00FF);
    uint16_t hi = cpu->readMemory((t + 1) & 0x00FF);

    addr_abs = (hi << 8) | lo;
    addr_abs += cpu->getRegisterY();

#if defined DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} HI:{:02X} LO:{:02X} {: >45}",
      getOperation(), addr_abs, hi, lo, cpu->reg
    );
#endif

    std::vector<uint8_t> ignoredOpCodes;
    ignoredOpCodes.push_back(0x91);
    if ((addr_abs & 0xFF00) != (hi << 8) && !in_array<uint8_t>(cpu->opcode, ignoredOpCodes))
    {
      cpu->addExtraCycle();
#if defined DEBUG
      Logger::log()->debug(
        "OP {} - addr_abs (Page Boundary): {:04X} HI:{:02X} LO:{:02X} {: >40}",
        getOperation(), addr_abs, hi, lo, cpu->reg
      );
#endif
    }
#if defined LOGMODE
    else if (in_array<uint8_t>(cpu->opcode, ignoredOpCodes))
    {
      Logger::log()->info("OP {} - Caught ignored OPCode", getOperation());
    }
#endif
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
#if defined DEBUG
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

#if defined DEBUG
    Logger::log()->debug(
      "OP {} - addr_abs: {:04X} - addr_rel: {:04X} - PC: {:04X} - REL + PC: {:06X}",
      getOperation(), addr_abs, addr_rel, pc, (pc + addr_rel)
    );
#endif

    if ((addr_abs & 0xFF00) != (pc & 0xFF00))
    {
      cpu->incrementCycleCount();
#if defined DEBUG
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
#if defined DEBUG
    cpu->dumpRam(cpu->getProgramCounter()-1);
#endif

    cpu->pokeStack((pc >> 8) & 0x00FF);
    cpu->decrementStackPointer();
    cpu->incrementCycleCount();

    cpu->pokeStack(pc & 0x00FF);
    cpu->decrementStackPointer();
    cpu->incrementCycleCount();

    if (isBreak)
    {
      cpu->setFlag(cpu->B, true);
    }
    else
    {
      cpu->setFlag(cpu->B, false);
    }

    cpu->pokeStack(cpu->getRegisterSR());
    cpu->decrementStackPointer();
    cpu->incrementCycleCount();

    cpu->setFlag(cpu->I, true);
#if defined EMULATE65C02
    cpu->setFlag(cpu->D, true);
#endif

    uint16_t newPc = (cpu->readMemory(vector + 1) << 8) | cpu->readMemory(vector);

//#if defined DEBUG
//    Logger::log()->debug("OP {} - NEW PC: {:04X} {: >59}", getOperation(), newPc, cpu->reg);
//#endif

#if defined DEBUG
    cpu->dumpStackAtPointer();
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
  void Executioner::opADC()
  {
    // Grab the data that we are adding to the accumulator
    fetch();

    uint8_t current = cpu->getRegisterAC();

    if (cpu->getFlag(cpu->D))
    {
      uint8_t d0 = (fetched & 0x0F) + (current & 0x0F) + (uint8_t)cpu->getFlag(cpu->C);
      uint8_t d1 = (fetched >> 4) + (current >> 4) + (d0 > 9 ? 1 : 0);

      temp = d0 % 10 | (d1 % 10 << 4);

      cpu->setFlag(cpu->C, d1 > 9);
    }
    else
    {
      // Add is performed in 16-bit domain for emulation to capture any
      // carry bit, which will exist in bit 8 of the 16-bit word
      temp = (uint16_t)current + (uint16_t)fetched + (uint16_t)cpu->getFlag(cpu->C);

      // The signed Overflow flag is set based on all that up there! :D
      cpu->setFlag(cpu->V, (~((uint16_t)fetched ^ (uint16_t)current) & ((uint16_t)temp ^ (uint16_t)current)) & 0x80);

      cpu->setFlag(cpu->C, temp > 255);
      temp = temp & 0x00FF;
    }

    // The Zero flag is set if the result is 0
    cpu->setFlag(cpu->Z, (temp & 0x00FF) == 0);

    // The negative flag is set to the most significant bit of the result
    cpu->setFlag(cpu->N, (temp & 0x80) > 0);

    // Load the result into the accumulator (it's 8-bit dont forget!)
    cpu->setRegisterAC((uint8_t)(temp & 0x00FF));
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
  void Executioner::opSBC()
  {
    fetch();

    uint8_t current = cpu->getRegisterAC();
    uint16_t value = 0x00;

    if (cpu->getFlag(cpu->D))
    {
      int8_t d0 = (current & 0x0F) - (fetched & 0x0F) - (cpu->getFlag(cpu->C) ? 0 : 1);
      int8_t d1 = (current >> 4) - (fetched >> 4) - (d0 < 0 ? 1 : 0);

      value = (d0 < 0 ? 10 + d0 : d0) | ((d1 < 0 ? 10 + d1 : d1) << 4);
      cpu->setRegisterAC((uint8_t)(value & 0xFF));

      cpu->setFlag(cpu->C, d1 < 0);
    }
    else
    {
      // Operating in 16-bit domain to capture carry out

      // We can invert the bottom 8 bits with bitwise xor
      uint16_t bottom = ((uint16_t)fetched) ^ 0x00FF;

      // Notice this is exactly the same as addition from here!
      value = (uint16_t)current + bottom + (uint16_t)cpu->getFlag(cpu->C);

      cpu->setFlag(cpu->V, (value ^ (uint16_t)current) & (value ^ bottom) & 0x0080);
      cpu->setFlag(cpu->C, value & 0xFF00);

      cpu->setRegisterAC((uint8_t)(value & 0xFF));
    }

    cpu->setFlag(cpu->Z, (value & 0x00FF) == 0);
    cpu->setFlag(cpu->N, value & 0x0080);
  }


  // OK! Complicated operations are done! the following are much simpler
  // and conventional. The typical order of events is:
  // 1) Fetch the data you are working with
  // 2) Perform calculation
  // 3) Store the result in desired place
  // 4) set Flags of the status register
  // 5) Return if instruction has potential to require additional 
  //    clock cycle

  // Instruction: Bitwise Logic AND
  // Function:    A = A & M
  // Flags Out:   N, Z
  void Executioner::opAND()
  {
    fetch();
    uint8_t value = (cpu->getRegisterAC() & fetched);
    cpu->setRegisterAC(value);
    cpu->setFlag(cpu->Z, value == 0x00);
    cpu->setFlag(cpu->N, value & 0x80);
  }


  // Instruction: Arithmetic Shift Left
  // Function:    A = C <- (A << 1) <- 0
  // Flags Out:   N, Z, C
  void Executioner::opASL()
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
    cpu->setFlag(cpu->C, (value & 0xFF00) > 0);
    cpu->setFlag(cpu->Z, (value & 0x00FF) == 0x00);
    cpu->setFlag(cpu->N, value & 0x80);

    if (getAddressModeName() == "ACC")
    {
      cpu->setRegisterAC((uint8_t)(value & 0x00FF));
    }
    else
    {
      cpu->writeMemory(addr_abs, (uint8_t)(value & 0x00FF));
    }

    if (getAddressModeName() == "ABX")
    {
#ifndef EMULATE65C02
      cpu->incrementCycleCount();
#endif
    }
  }


  // Instruction: Branch if Carry Clear
  // Function:    if(C == 0) pc = address 
  void Executioner::opBCC()
  {
    branchOperation(cpu->getFlag(cpu->C) == 0);
  }


  // Instruction: Branch if Carry set
  // Function:    if(C == 1) pc = address
  void Executioner::opBCS()
  {
    branchOperation(cpu->getFlag(cpu->C) == 1);
  }


  // Instruction: Branch if Equal
  // Function:    if(Z == 1) pc = address
  void Executioner::opBEQ()
  {
    branchOperation(cpu->getFlag(cpu->Z) == 1);
  }


  // Instruction: Test Bits in Memory with Accumulator
  // Function:    A & M, M7 -> N, M6 -> V
  // Flags Out:   N, Z, V
  void Executioner::opBIT()
  {
    fetch();

    uint8_t value = (cpu->getRegisterAC() & fetched);

    cpu->setFlag(cpu->Z, (value & 0xFF) == 0x00);
    cpu->setFlag(cpu->N, fetched & (1 << 7));
    cpu->setFlag(cpu->V, fetched & (1 << 6));
  }


  // Instruction: Branch if Negative
  // Function:    if(N == 1) pc = address
  void Executioner::opBMI()
  {
    branchOperation(cpu->getFlag(cpu->N) == 1);
  }


  // Instruction: Branch if Not Equal
  // Function:    if(Z == 0) pc = address
  void Executioner::opBNE()
  {
    branchOperation(cpu->getFlag(cpu->Z) == 0);
  }


  // Instruction: Branch if Positive
  // Function:    if(N == 0) pc = address
  void Executioner::opBPL()
  {
    branchOperation(cpu->getFlag(cpu->N) == 0);
  }


  // Instruction: Break
  // Function:    Program Sourced Interrupt
  void Executioner::opBRK()
  {
    breakOperation(true, 0xFFFE);
  }


  // Instruction: Branch if Overflow Clear
  // Function:    if(V == 0) pc = address
  void Executioner::opBVC()
  {
    branchOperation(cpu->getFlag(cpu->V) == 0);
  }


  // Instruction: Branch if Overflow set
  // Function:    if(V == 1) pc = address
  void Executioner::opBVS()
  {
    branchOperation(cpu->getFlag(cpu->V) == 1);
  }


  // Instruction: Clear Carry Flag
  // Function:    C = 0
  void Executioner::opCLC()
  {
    cpu->setFlag(cpu->C, false);
    cpu->incrementCycleCount();
  }


  // Instruction: Clear Decimal Flag
  // Function:    D = 0
  void Executioner::opCLD()
  {
    cpu->setFlag(cpu->D, false);
    cpu->incrementCycleCount();
  }


  // Instruction: Disable Interrupts / Clear Interrupt Flag
  // Function:    I = 0
  void Executioner::opCLI()
  {
    cpu->setFlag(cpu->I, false);
    cpu->incrementCycleCount();
  }


  // Instruction: Clear Overflow Flag
  // Function:    V = 0
  void Executioner::opCLV()
  {
    cpu->setFlag(cpu->V, false);
    cpu->incrementCycleCount();
  }

  // Instruction: Compare Accumulator
  // Function:    C <- A >= M      Z <- (A - M) == 0
  // Flags Out:   N, C, Z
  void Executioner::opCMP()
  {
    fetch();

    uint8_t value = (cpu->getRegisterAC() - fetched);
    cpu->setFlag(cpu->C, cpu->getRegisterAC() >= fetched);
    cpu->setFlag(cpu->Z, (value & 0x00FF) == 0x0000);
    cpu->setFlag(cpu->N, value & 0x0080);
  }


  // Instruction: Compare X Register
  // Function:    C <- X >= M      Z <- (X - M) == 0
  // Flags Out:   N, C, Z
  void Executioner::opCPX()
  {
    fetch();

    uint8_t value = (cpu->getRegisterX() - fetched);
    cpu->setFlag(cpu->C, cpu->getRegisterX() >= fetched);
    cpu->setFlag(cpu->Z, (value & 0x00FF) == 0x0000);
    cpu->setFlag(cpu->N, value & 0x0080);
  }


  // Instruction: Compare Y Register
  // Function:    C <- Y >= M      Z <- (Y - M) == 0
  // Flags Out:   N, C, Z
  void Executioner::opCPY()
  {
    fetch();

    uint8_t value = (cpu->getRegisterY() - fetched);
    cpu->setFlag(cpu->C, cpu->getRegisterY() >= fetched);
    cpu->setFlag(cpu->Z, (value & 0x00FF) == 0x0000);
    cpu->setFlag(cpu->N, value & 0x0080);
  }


  // Instruction: Decrement Value at Memory Location
  // Function:    M = M - 1
  // Flags Out:   N, Z
  void Executioner::opDEC()
  {
    fetch();

    cpu->writeMemory(addr_abs, fetched & 0x00FF);
    temp = fetched - 1;
    cpu->setFlag(cpu->Z, (temp & 0x00FF) == 0x0000);
    cpu->setFlag(cpu->N, temp & 0x0080);

    cpu->writeMemory(addr_abs, temp & 0x00FF);

    if (getAddressModeName() == "ABX")
    {
      cpu->incrementCycleCount();
    }
  }


  // Instruction: Decrement X Register
  // Function:    X = X - 1
  // Flags Out:   N, Z
  void Executioner::opDEX()
  {
    uint8_t value = cpu->getRegisterX();
    --value;
    cpu->setRegisterX(value);
    cpu->setFlag(cpu->Z, (value & 0xFF) == 0x00);
    cpu->setFlag(cpu->N, value & 0x80);

    cpu->incrementCycleCount();
  }


  // Instruction: Decrement Y Register
  // Function:    Y = Y - 1
  // Flags Out:   N, Z
  void Executioner::opDEY()
  {
    uint8_t value = cpu->getRegisterY();
    --value;
    cpu->setRegisterY(value);
    cpu->setFlag(cpu->Z, (value & 0xFF) == 0x00);
    cpu->setFlag(cpu->N, value & 0x80);

    cpu->incrementCycleCount();
  }


  // Instruction: Bitwise Logic XOR
  // Function:    A = A xor M
  // Flags Out:   N, Z
  void Executioner::opEOR()
  {
    fetch();

    uint8_t value = cpu->getRegisterAC() ^ fetched;
    cpu->setRegisterAC(value);
    cpu->setFlag(cpu->Z, (value & 0xFF) == 0x00);
    cpu->setFlag(cpu->N, value & 0x80);
  }


  // Instruction: Increment Value at Memory Location
  // Function:    M = M + 1
  // Flags Out:   N, Z
  void Executioner::opINC()
  {
    fetch();

    cpu->writeMemory(addr_abs, fetched & 0x00FF);
    temp = fetched + 1;

    cpu->setFlag(cpu->Z, (temp & 0x00FF) == 0x0000);
    cpu->setFlag(cpu->N, temp & 0x0080);

    cpu->writeMemory(addr_abs, temp & 0x00FF);

    if (getAddressModeName() == "ABX")
    {
      cpu->incrementCycleCount();
    }
  }


  // Instruction: Increment X Register
  // Function:    X = X + 1
  // Flags Out:   N, Z
  void Executioner::opINX()
  {
    uint8_t value = cpu->getRegisterX();
    ++value;
    cpu->setRegisterX(value);
    cpu->setFlag(cpu->Z, (value & 0xFF) == 0x00);
    cpu->setFlag(cpu->N, value & 0x80);

    cpu->incrementCycleCount();
  }


  // Instruction: Increment Y Register
  // Function:    Y = Y + 1
  // Flags Out:   N, Z
  void Executioner::opINY()
  {
    uint8_t value = cpu->getRegisterY();
    ++value;
    cpu->setRegisterY(value);
    cpu->setFlag(cpu->Z, (value & 0xFF) == 0x00);
    cpu->setFlag(cpu->N, value & 0x80);

    cpu->incrementCycleCount();
  }


  // Instruction: Jump To Location
  // Function:    pc = address
  void Executioner::opJMP()
  {
    cpu->setProgramCounter(addr_abs);
  }


  // Instruction: Jump To Sub-Routine
  // Function:    Push current pc to stack, pc = address
  void Executioner::opJSR()
  {
    cpu->incrementCycleCount();
    cpu->decrementProgramCounter();

    uint16_t pc = cpu->getProgramCounter();

    cpu->pokeStack((pc >> 8) & 0x00FF);
    cpu->incrementCycleCount();
    cpu->decrementStackPointer();

    cpu->pushStack(pc & 0x00FF);

    cpu->setProgramCounter(addr_abs);
  }


  // Instruction: Load The Accumulator
  // Function:    A = M
  // Flags Out:   N, Z
  void Executioner::opLDA()
  {
    fetch();

    cpu->setRegisterAC(fetched);
    cpu->setFlag(cpu->Z, fetched == 0x00);
    cpu->setFlag(cpu->N, fetched & 0x80);
  }


  // Instruction: Load The X Register
  // Function:    X = M
  // Flags Out:   N, Z
  void Executioner::opLDX()
  {
    fetch();

    cpu->setRegisterX(fetched);
    cpu->setFlag(cpu->Z, fetched == 0x00);
    cpu->setFlag(cpu->N, fetched & 0x80);
  }


  // Instruction: Load The Y Register
  // Function:    Y = M
  // Flags Out:   N, Z
  void Executioner::opLDY()
  {
    fetch();

    cpu->setRegisterY(fetched);
    cpu->setFlag(cpu->Z, fetched == 0x00);
    cpu->setFlag(cpu->N, fetched & 0x80);
  }


  // Instruction: Logical Shift Right
  // Function:    A = C <- (A << 1) <- 0
  // Flags Out:   N=0, Z, C
  void Executioner::opLSR()
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

    cpu->setFlag(cpu->C, fetched & 0x0001);
    temp = fetched >> 1;
    cpu->setFlag(cpu->Z, (temp & 0x00FF) == 0x0000);
    cpu->setFlag(cpu->N, temp & 0x0080);


    uint8_t value = temp & 0x00FF;

    if (getAddressModeName() == "ACC")
    {
      cpu->setRegisterAC(value);
    }
    else
    {
      cpu->writeMemory(addr_abs, value);
    }

    if (getAddressModeName() == "ABX")
    {
#ifndef EMULATE65C02
      cpu->incrementCycleCount();
#endif
    }
  }


  // Instruction: No Operation
  // Function:    -
  void Executioner::opNOP()
  {
    cpu->incrementCycleCount();
  }


  // Instruction: Bitwise Logic OR
  // Function:    A = A | M
  // Flags Out:   N, Z
  void Executioner::opORA()
  {
    fetch();

    uint8_t value = cpu->getRegisterAC() | fetched;
    cpu->setRegisterAC(value);
    cpu->setFlag(cpu->Z, value == 0x00);
    cpu->setFlag(cpu->N, value & 0x80);
  }


  // Instruction: Push Accumulator to Stack
  // Function:    A -> stack
  void Executioner::opPHA()
  {
    uint8_t ac = cpu->getRegisterAC();
    cpu->pushStack(ac);
    cpu->incrementCycleCount();
    Logger::log()->debug("OP {} - newSR: {} {: >56}", getOperation(), cpu->decodeFlag(ac), cpu->reg);
  }


  // Instruction: Push Status Register to Stack
  // Function:    status -> stack
  // Note:        Break flag is set to 1 before push
  void Executioner::opPHP()
  {
    uint8_t status = cpu->getRegisterSR() | cpu->B | cpu->U;

    cpu->pushStack(status);
    cpu->incrementCycleCount();
  }


  // Instruction: Pop Accumulator off Stack
  // Function:    A <- stack
  // Flags Out:   N, Z
  void Executioner::opPLA()
  {
    uint8_t value = cpu->popStack();
    cpu->incrementCycleCount();
    cpu->setRegisterAC(value);
    cpu->setFlag(cpu->Z, value == 0x00);
    cpu->setFlag(cpu->N, value & 0x80);
    cpu->incrementCycleCount();
  }


  // Instruction: Pop Status Register off Stack
  // Function:    Status <- stack
  void Executioner::opPLP()
  {
    cpu->setRegisterSR(cpu->popStack());
    cpu->incrementCycleCount();

    cpu->setFlag(cpu->U, 1);
    cpu->incrementCycleCount();
  }


  // Instruction: Rotate Left
  // Function:    (C << 1)
  // Flags Out:    N, Z, C
  void Executioner::opROL()
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

    temp = (uint16_t)(fetched << 1) | cpu->getFlag(cpu->C);
    cpu->setFlag(cpu->C, temp & 0xFF00);
    cpu->setFlag(cpu->Z, (temp & 0x00FF) == 0x0000);
    cpu->setFlag(cpu->N, temp & 0x0080);

    if (getAddressModeName() == "ACC")
    {
      cpu->setRegisterAC((uint8_t)(temp & 0x00FF));
    }
    else
    {
      cpu->writeMemory(addr_abs, temp & 0x00FF);
    }

    if (getAddressModeName() == "ABX")
    {
#ifndef EMULATE65C02
      cpu->incrementCycleCount();
#endif
    }
  }


  // Instruction: Rotate Right
  // Function:    (C >> 1)
  // Flags Out:    N, Z, C
  void Executioner::opROR()
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

    temp = (uint16_t)(cpu->getFlag(cpu->C) << 7) | (fetched >> 1);
    cpu->setFlag(cpu->C, fetched & 0x01);
    cpu->setFlag(cpu->Z, (temp & 0x00FF) == 0x00);
    cpu->setFlag(cpu->N, temp & 0x0080);

    if (getAddressModeName() == "ACC")
    {
      cpu->setRegisterAC((uint8_t)(temp & 0x00FF));
    }
    else
    {
      cpu->writeMemory(addr_abs, temp & 0x00FF);
    }

    if (getAddressModeName() == "ABX")
    {
#ifndef EMULATE65C02
      cpu->incrementCycleCount();
#endif
    }
  }


  // Instruction: Return from Interrupt
  // Function:    Pull SR, Pull PC
  // Flags Out:    From Stack
  void Executioner::opRTI()
  {
    cpu->incrementCycleCount();

    cpu->incrementStackPointer();
    cpu->incrementCycleCount();

    uint8_t newSR = cpu->peekStack();
    cpu->incrementCycleCount();

    cpu->setRegisterSR(newSR & ~cpu->B & ~cpu->U);

    uint16_t pcLo = cpu->popStack();

    cpu->incrementStackPointer();
    uint16_t pcHi = cpu->peekStack() << 8;
    
    uint16_t newPc = (pcHi | pcLo);
    cpu->incrementCycleCount();

    cpu->setProgramCounter(newPc);
  }


  // Instruction: Return from Subroutine
  // Function:    Pull PC, PC+1 -> PC
  // Flags Out:    -
  void Executioner::opRTS()
  {
    cpu->incrementCycleCount();
    uint16_t newPc = cpu->popStack() | (cpu->popStack() << 8);

    cpu->incrementCycleCount();
    cpu->setProgramCounter(newPc);


    cpu->incrementProgramCounter();
    cpu->incrementCycleCount();
  }


  // Instruction: set Carry Flag
  // Function:    C = 1
  void Executioner::opSEC()
  {
    cpu->setFlag(cpu->C, true);
    cpu->incrementCycleCount();
  }


  // Instruction: set Decimal Flag
  // Function:    D = 1
  void Executioner::opSED()
  {
    cpu->setFlag(cpu->D, true);
    cpu->incrementCycleCount();
  }


  // Instruction: set Interrupt Flag / Enable Interrupts
  // Function:    I = 1
  void Executioner::opSEI()
  {
    cpu->setFlag(cpu->I, true);
    cpu->incrementCycleCount();
  }


  // Instruction: Store Accumulator at Address
  // Function:    M = A
  void Executioner::opSTA()
  {
    cpu->writeMemory(addr_abs, cpu->getRegisterAC());

    std::vector<std::string> affectedAddrModes;
    affectedAddrModes.push_back("ABX");
    affectedAddrModes.push_back("ABY");
    affectedAddrModes.push_back("IZY");

    if (in_array<std::string>(getAddressModeName(), affectedAddrModes))
    {
      cpu->incrementCycleCount();
    }
  }


  // Instruction: Store X Register at Address
  // Function:    M = X
  void Executioner::opSTX()
  {
    cpu->writeMemory(addr_abs, cpu->getRegisterX());
  }


  // Instruction: Store Y Register at Address
  // Function:    M = Y
  void Executioner::opSTY()
  {
    cpu->writeMemory(addr_abs, cpu->getRegisterY());
  }


  // Instruction: Transfer Accumulator to X Register
  // Function:    X = A
  // Flags Out:   N, Z
  void Executioner::opTAX()
  {
    cpu->incrementCycleCount();

    uint8_t value = cpu->getRegisterAC();
    cpu->setRegisterX(value);

    cpu->setFlag(cpu->Z, value == 0x00);
    cpu->setFlag(cpu->N, value & 0x80);
  }


  // Instruction: Transfer Accumulator to Y Register
  // Function:    Y = A
  // Flags Out:   N, Z
  void Executioner::opTAY()
  {
    cpu->incrementCycleCount();

    uint8_t value = cpu->getRegisterAC();
    cpu->setRegisterY(value);

    cpu->setFlag(cpu->Z, value == 0x00);
    cpu->setFlag(cpu->N, value & 0x80);
  }


  // Instruction: Transfer Stack Pointer to X Register
  // Function:    X = stack pointer
  // Flags Out:   N, Z
  void Executioner::opTSX()
  {
    uint8_t value = cpu->getRegisterSP();
    cpu->setRegisterX(value);

    cpu->setFlag(cpu->Z, value == 0x00);
    cpu->setFlag(cpu->N, value & 0x80);
    cpu->incrementCycleCount();
  }


  // Instruction: Transfer X Register to Accumulator
  // Function:    A = X
  // Flags Out:   N, Z
  void Executioner::opTXA()
  {
    cpu->incrementCycleCount();
    uint8_t value = cpu->getRegisterX();
    cpu->setRegisterAC(value);

    cpu->setFlag(cpu->Z, value == 0x00);
    cpu->setFlag(cpu->N, value & 0x80);
  }


  // Instruction: Transfer X Register to Stack Pointer
  // Function:    stack pointer = X
  void Executioner::opTXS()
  {
    uint8_t value = cpu->getRegisterX();
    cpu->setRegisterSP(value);
    cpu->incrementCycleCount();
  }


  // Instruction: Transfer Y Register to Accumulator
  // Function:    A = Y
  // Flags Out:   N, Z
  void Executioner::opTYA()
  {
    cpu->incrementCycleCount();

    uint8_t value = cpu->getRegisterY();
    cpu->setRegisterAC(value);
    cpu->setFlag(cpu->Z, value == 0x00);
    cpu->setFlag(cpu->N, value & 0x80);
  }

#if defined EMULATE65C02
  void Executioner::opSTP()
  {

  }

  void Executioner::opWAI()
  {

  }
#endif

#if defined ILLEGAL
  // Illegal opcodes

  // Instruction: AND oper + LSR
  // Function:    A AND oper, 0 -> [76543210] -> C
  // Flags Out:   N, Z, C
  // Note:        AKA ASR
  void Executioner::opALR()
  {
    fetch();

    temp = cpu->getRegisterAC() & fetched;
    cpu->setRegisterAC((uint8_t)(temp >> 1));

    cpu->setFlag(cpu->Z, temp == 0x00);
    cpu->setFlag(cpu->N, temp & 0x80);
    cpu->setFlag(cpu->C, temp & 0x0001);
  }


  // Instruction: AND oper + set C as ASL
  // Function:    A AND oper, bit(7) -> C
  // Flags Out:   N, Z, C
  // OpCode:      0x0B
  void Executioner::opANC()
  {
    fetch();

    uint8_t value = cpu->getRegisterAC() & fetched;
    cpu->setRegisterAC(value);

    cpu->setFlag(cpu->Z, value == 0x00);
    cpu->setFlag(cpu->N, value & 0x80);
    cpu->setFlag(cpu->C, (value & 0xFF00) > 0);
  }

  // Instruction: AND oper + set C as ROL
  // Function:    A AND oper, bit(7) -> C
  // Flags Out:   N, Z, C
  // OpCode:      0x2B
  // @see OPCode::ANC
  void Executioner::opANC2()
  {
    fetch();

    uint8_t value = cpu->getRegisterAC() & fetched;
    cpu->setRegisterAC(value);

    cpu->setFlag(cpu->Z, value == 0x00);
    cpu->setFlag(cpu->N, value & 0x80);
    cpu->setFlag(cpu->C, value & 0xFF00);
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
  void Executioner::opANE()
  {
    fetch();

    uint8_t ac_value = cpu->getRegisterAC();
    uint8_t x_value = cpu->getRegisterX();


    ac_value = (ac_value ^ magic_ANE) & x_value & fetched;

    cpu->setRegisterAC(ac_value);

    cpu->setFlag(cpu->Z, (ac_value & 0x00FF) == 0x00);
    cpu->setFlag(cpu->N, ac_value & 0x0080);
  }


  // Instruction: AND oper + ROR
  // Function:    A AND oper, C -> [76543210] -> C
  // Flags Out:   N, Z, C, V
  void Executioner::opARR()
  {
    fetch();

    uint8_t value = cpu->getRegisterAC();
    uint8_t carry = cpu->getFlag(cpu->C);
    uint8_t tempSR = cpu->getRegisterSR();
    tempSR &= ~(cpu->C | cpu->Z | cpu->V | cpu->N);
    cpu->setRegisterSR(tempSR);
    uint8_t result;

    if (tempSR & carry)
    {
      result = (value & 0x0F) + (fetched & 0x0F) + carry;
      if (result > 0x9)
      {
        result += 0x6;
      }
      if (result <= 0xF)
      {
        result = (result & 0x0F) + (value & 0xF0) + (fetched & 0xF0);
      }
      else
      {
        result = (result & 0x0F) + (value & 0xF0) + (result & 0xF0) + 0x10;
      }

      cpu->setFlag(cpu->Z, ((value + fetched + carry) & 0x00FF) == 0x00);
      cpu->setFlag(cpu->N, result & 0x0080);
      cpu->setFlag(cpu->V, ((value ^ result) & 0x80) && !((value ^ result) & 0x80));
      if ((result & 0x1F0) > 0x90)
      {
        result += 0x60;
      }
      cpu->setFlag(cpu->C, result & 0xFF0);
    }
    else
    {
      result = value + fetched;
      cpu->setFlag(cpu->C, ((result & 0x80) >> 7));
      cpu->setFlag(cpu->V, ((result & 0x40) >> 6) ^ ((result & 0x80) >> 7));

      result = (result >> 1) | (carry << 7);
      cpu->setFlag(cpu->Z, (result & 0x00FF) == 0x00);
      cpu->setFlag(cpu->N, result & 0x0080);

    }

    if (getAddressModeName() == "IMP" || getAddressModeName() == "IMM")
    {
      cpu->setRegisterAC((uint8_t)(result & 0x00FF));
    }
    else
    {
      cpu->writeMemory(addr_abs, result & 0x00FF);
    }
  }


  // Instruction: DEC oper + CMP oper
  // Function:    M - 1 -> M, A - M
  // Flags Out:   N, Z, C
  void Executioner::opDCP()
  {
    fetch();

    //uint8_t value = cpu->getRegisterAC();
    //temp = fetched - 1;

    uint8_t result = (fetched + 0xFF) & 0xFF;
    uint8_t diff = cpu->getRegisterAC() + (~result & 0xFF) + 1;
    cpu->incrementCycleCount();

    //cpu->writeMemory(addr_abs, temp & 0x00FF);
    cpu->setFlag(cpu->N, temp & 0x80);
    cpu->setFlag(cpu->Z, (temp & 0xFF) == 0x00);
    cpu->setFlag(cpu->C, diff >> 8);

    if (getAddressModeName() == "ABX" || getAddressModeName() == "IZY")
    {
      cpu->incrementCycleCount();
    }

    cpu->writeMemory(addr_abs, result & 0x00FF);
  }


  // Instruction: INC oper + SBC oper
  // Function:    M + 1 -> M, A - M - (C - 1) -> A
  // Flags Out:   N, Z, C, V
  void Executioner::opISC()
  {
    fetch();
    temp = fetched + 1;
    //cpu->writeMemory(addr_abs, temp);
    cpu->writeMemory(addr_abs, temp & 0x00FF);
    //cpu->incrementCycleCount();

    if (getAddressModeName() == "ABX" || getAddressModeName() == "ABY" || getAddressModeName() == "IZY")
    {
      cpu->incrementCycleCount();
    }

    opSBC();
  }


  // Instruction: LDA/TSX oper
  // Function:    M AND SP -> A, X, SP
  // Flags Out:   N, Z
  void Executioner::opLAS()
  {
    fetch();
    uint8_t result = cpu->getRegisterSP() & fetched;
    cpu->setRegisterSP(result);
    cpu->setRegisterAC(result);
    cpu->setRegisterX(result);

    cpu->setFlag(cpu->Z, (result & 0xFF) == 0x00);
    cpu->setFlag(cpu->N, result & 0x80);
  }


  // Instruction: LDA oper + LDX oper
  // Function:    M -> A -> X
  // Flags Out:   N, Z
  void Executioner::opLAX()
  {
    fetch();

    cpu->setRegisterAC(fetched);
    cpu->setRegisterX(fetched);

    cpu->setFlag(cpu->Z, (fetched & 0xFF) == 0x00);
    cpu->setFlag(cpu->N, fetched & 0x80);
  }


  // Instruction: Store * AND oper in A and X
  // Function:    (A OR CONST) AND oper -> A -> X
  // Flags Out:   N, Z
  // Note:        Highly unstable, involves a "magic" constant
  // See:         Executioner::ANE
  void Executioner::opLXA()
  {
    fetch();

    uint8_t value = (cpu->getRegisterAC() ^ magic_LXA) & fetched;

    cpu->setRegisterAC(value);
    cpu->setRegisterX(value);

    cpu->setFlag(cpu->Z, (value & 0x00FF) == 0x00);
    cpu->setFlag(cpu->N, value & 0x0080);
  }


  // Instruction: ROL oper + AND oper
  // Function:    M = C <- [76543210] <- C, A AND M -> A
  // Flags Out:   N, Z, C
  void Executioner::opRLA()
  {
    fetch();
    temp = (uint16_t)((fetched << 1) & 0xFF) | cpu->getFlag(cpu->C);
    uint8_t ac = cpu->getRegisterAC();
    //cpu->writeMemory(addr_abs, temp & 0x00FF);
    ac &= temp;
    cpu->incrementCycleCount();
    cpu->setFlag(cpu->C, fetched >> 7);
    cpu->setFlag(cpu->N, ac & 0x80);
    cpu->setFlag(cpu->Z, (ac & 0xFF) == 0x00);

    cpu->incrementCycleCount();
    if (getAddressModeName() == "ABX" || getAddressModeName() == "ABY" || getAddressModeName() == "IZY")
    {
      cpu->incrementCycleCount();
    }

    //opAND();
  }


  // Instruction: ROL oper + ADC oper
  // Function:    M = C -> [76543210] -> C, A + M + C -> A, C
  // Flags Out:   N, Z, C, V
  void Executioner::opRRA()
  {
    fetch();
    temp = (uint16_t)(fetched << 1) | cpu->getFlag(cpu->C);
    cpu->writeMemory(addr_abs, temp & 0x00FF);
    cpu->setFlag(cpu->C, (fetched & 0xFF00) > 0);
    opADC();
  }


  // Instruction: A and X are put on the bus at the same
  //              time (resulting effectively in an AND
  //              operation) and stored in M
  // Function:    A AND X -> M
  // Flags Out:   -
  void Executioner::opSAX()
  {
    fetch();

    uint8_t value = cpu->getRegisterAC() & cpu->getRegisterX();
    //cpu->writeMemory(addr_abs, value & 0x00FF);
    cpu->setFlag(cpu->Z, value == 0x00);
    cpu->setFlag(cpu->N, value & 0x80);
    //cpu->incrementCycleCount();
  }


  // Instruction: CMP and DEX at once, sets flags like CMP
  // Function:    (A AND X) - oper -> X
  // Flags Out:   N, Z, C
  void Executioner::opSBX()
  {
    fetch();

    uint8_t value = (cpu->getRegisterAC() & cpu->getRegisterX()) - fetched;
    cpu->setRegisterX(value);
    //x = ((uint16_t)a & (uint16_t)x) - (uint16_t)fetched;
    cpu->setFlag(cpu->C, value & 0xFF00);
    cpu->setFlag(cpu->Z, (value & 0x00FF) == 0x0000);
    cpu->setFlag(cpu->N, value & 0x0080);
  }


  // Instruction: Stores A AND X AND (high-byte of addr. + 1) at addr.
  // Function:    A AND X AND (H+1) -> M
  // Flags Out:   -
  // Note:        Unstable: Sometimes 'AND (H+1)' is dropped, page boundary
  //              crossings may not work (with the high-byte of the value used
  //              as the high-byte of the address).
  void Executioner::opSHA()
  {
    fetch();
    //temp = ((uint16_t)a & (uint16_t)x) & (uint16_t)((addr_abs >> 8) + 1);
    //cpu->writeMemory(addr_abs, temp & 0x00FF);

    uint16_t value = (cpu->getRegisterAC() & cpu->getRegisterX());
    value &= (uint16_t)((addr_abs >> 8) + 1);
    cpu->writeMemory(addr_abs, (uint8_t)(temp & 0x00FF));
  }


  // Instruction: Stores X AND (high-byte of addr. + 1) at addr.
  // Function:    X AND (H+1) -> M
  // Flags Out:   -
  void Executioner::opSHX()
  {
    fetch();
    //temp = ((uint16_t)x) & (uint16_t)((addr_abs >> 8) + 1);
    //cpu->writeMemory(addr_abs, temp & 0x00FF);

    uint16_t value = (cpu->getRegisterX() & ((addr_abs >> 8) + 1));
    cpu->writeMemory(addr_abs, (uint8_t)(temp & 0x00FF));
  }


  // Instruction: Stores Y AND (high-byte of addr. + 1) at addr.
  // Function:    Y AND (H+1) -> M
  // Flags Out:   -
  void Executioner::opSHY()
  {
    fetch();
    //temp = ((uint16_t)y) & (uint16_t)((addr_abs >> 8) + 1);
    //cpu->writeMemory(addr_abs, temp & 0x00FF);

    uint16_t value = (cpu->getRegisterY() & ((addr_abs >> 8) + 1));
    cpu->writeMemory(addr_abs, (uint8_t)(temp & 0x00FF));
  }


  // Instruction: ASL oper + ORA oper
  // Function:    M = C <- [76543210] <- 0, A OR M -> A
  // Flags Out:   N, Z, C
  void Executioner::opSLO()
  {
    fetch();
    temp = (uint16_t)fetched << 1;
    cpu->setFlag(cpu->C, (temp & 0xFF00) > 0);
    cpu->writeMemory(addr_abs, temp & 0x00FF);

    cpu->incrementCycleCount();
    //a = a | fetched;
    uint8_t value = cpu->getRegisterAC() | fetched;
    cpu->setFlag(cpu->Z, (value & 0xFF) == 0x00);
    cpu->setFlag(cpu->N, value & 0x80);
    if (getAddressModeName() == "ABX" || getAddressModeName() == "ABY" || getAddressModeName() == "IZY")
    {
      cpu->incrementCycleCount();
    }
  }


  // Instruction: LSR oper + EOR oper
  // Function:    M = 0 -> [76543210] -> 0, A EOR M -> A
  // Flags Out:   -
  void Executioner::opSRE()
  {
    fetch();

    cpu->setFlag(cpu->C, fetched & 0x0001);
    temp = fetched >> 1;
    cpu->setFlag(cpu->Z, (temp & 0xFF) == 0x0000);
    cpu->setFlag(cpu->N, temp & 0x80);

    uint8_t value = (uint8_t)(temp & 0xFF);

    if (getAddressModeName() == "IMP")
    {
      cpu->setRegisterAC(value);
    }
    else
    {
      cpu->writeMemory(addr_abs, value);
    }

    if (getAddressModeName() == "ABX" || getAddressModeName() == "IZY" || getAddressModeName() == "ABY")
    {
      cpu->incrementCycleCount();
    }
    cpu->incrementCycleCount();

    cpu->setRegisterAC(value ^ fetched);
  }


  // Instruction: Puts A AND X in SP and stores A AND X AND (high-byte of addr. + 1) at addr.
  // Function:    A AND X -> SP, A AND X AND (H + 1) -> M
  // Flags Out:   -
  void Executioner::opTAS()
  {
    fetch();
    uint8_t value = cpu->getRegisterAC() & cpu->getRegisterX();
    cpu->setRegisterSP(value);

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
  }


  // Alias for SBC
  void Executioner::opUSBC()
  {
    return opSBC();
  }


  // Instruction: No Operation (Skip Byte)
  // Function:    -
  // Flags Out:   -
  // Note:        -
  void Executioner::opDOP()
  {
    // Sadly not all NOPs are equal, Ive added a few here
    // based on https://wiki.nesdev.com/w/index.php/CPU_unofficial_opcodes
    // and will add more based on game compatibility, and ultimately
    // I'd like to cover all illegal opcodes too
    //cpu->incrementCycleCount();
    cpu->incrementCycleCount();
  }


  // Instruction: No Operation (Ignore)
  // Function:    -
  // Flags Out:   -
  // Note:        -
  void Executioner::opTOP()
  {
    cpu->incrementCycleCount();
  }


  // This instruction freezes the CPU.
  // The processor will be trapped infinitely in
  // T1 phase with $FF on the data bus.
  // — Reset required.
  void Executioner::opJAM()
  {
    cpu->setJammed();
    breakOperation(false, cpu->getProgramCounter());
    //throw std::exception();
  }
#endif

  // This function is called when missing (aka. illegal) opcodes is called
  // Only needed when ILLEGAL macro is not set
  void Executioner::opXXX()
  {
    std::string msg = fmt::format("Instruction {:02X} is illegal", cpu->opcode);
    std::cout << msg << std::endl;
    //throw IllegalInstruction("Invalid Instruction");
  }

#pragma endregion INSTRUCTION IMPLEMENTATIONS
};
