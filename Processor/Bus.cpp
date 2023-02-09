#include "BaseBus.hpp"
#include "Types.hpp"
#include "Logger.hpp"

#include <iostream>
#include <signal.h>
#include <stdexcept>


namespace Processor
{
  void handler(int sig)
  {
    switch(sig)
    {
      case SIGABRT:
        std::cout << "Abort Detected." << std::endl;
        break;
      case SIGFPE:
        std::cout << "Floating Point Exception Detected." << std::endl;
        break;
      case SIGILL:
        std::cout << "Illegal Instruction Detected." << std::endl;
        break;
      case SIGINT:
        std::cout << "Interrupt Detected." << std::endl;
        break;
      case SIGSEGV:
        std::cout << "Segmention Fault Detected." << std::endl;
        break;
      case SIGTERM:
        std::cout << "Termination Detected." << std::endl;
        break;
      default:
        std::cout << "Unknown Fault Detected." << std::endl;
    }
#if defined DEBUG
    std::cout << "Printing Backtrace:" << std::endl << Backtrace() << std::endl;
#endif
    exit(128 + sig);
  }

  BaseBus::BaseBus()
  {
    try
    {
      signal(SIGABRT, handler);
      signal(SIGFPE, handler);
      signal(SIGILL, handler);
      signal(SIGINT, handler);
      signal(SIGSEGV, handler);
      signal(SIGTERM, handler);

      // Connect CPU to communication bus
      //cpu.connectBus(this);
      
      // Clear RAM contents, just in case :P
      reset();
    }
    catch (const std::exception& e)
    {
      std::cout << "std::exception :" << e.what() << std::endl;
#if defined DEBUG
      print_exception(e);
#endif
    }
    catch (...)
    {
      std::cout << "catch_all exception" << std::endl;
      exit(1);
    }
  }


  BaseBus::~BaseBus()
  {
  }

  void BaseBus::cpuWrite(uint16_t addr, uint8_t data)
  {
    if (addr >= 0x0000 && addr <= 0xFFFF)
    {
      ram[addr] = data;
    }
  }

  uint8_t BaseBus::cpuRead(uint16_t addr, bool bReadOnly)
  {
    if (addr >= 0x0000 && addr <= 0xFFFF)
    {
      return ram[addr];
    }
    
    return 0x00;
  }

  void BaseBus::reset()
  {
    for (auto& i : ram)
    {
      i = 0x00;
    }
  }

  void BaseBus::clock()
  {
  }

  bool BaseBus::complete()
  {
    return true;
  }

  void BaseBus::dump(uint16_t offset)
  {
#if defined LOGMODE
    Logger::log()->info("Actual ADDR: ${:04X}", offset);
    
    uint16_t offsetStart = offset & 0xFFF0;
    uint16_t offsetStop = offset | 0x000F;
    dump(offsetStart, offsetStop);
#endif
  }

  void BaseBus::dump(uint16_t offsetStart, uint16_t offsetStop)
  {
#if defined LOGMODE
    Logger::log()->info("MEMORY LOG FOR: ${:04X} - ${:04X}", offsetStart, offsetStop);
    Logger::log()->info(" ADDR 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
    
    std::map<uint16_t, BaseBus::MEMORYMAP> memory = memoryDump(offsetStart, offsetStop);
    for (auto& i : memory)
    {
      Logger::log()->info("{}", i.second);
    }
#endif
  }

/*
  std::string BaseBus::dumpRaw(uint16_t offsetStart, uint16_t offsetStop)
  {
    std::string log = fmt::format("MEMORY LOG FOR: ${:04X} - ${:04X} \n${:04X}:", offsetStart, offsetStop, offsetStart);
   
    uint16_t multiplier = 0;
    for (uint16_t i = offsetStart; i <= offsetStop; i++)
    {
      if (i % 16 == 0 && i != offsetStart) {
        multiplier++;
        if (i != offsetStop)
        {
          log += fmt::format("\n${:04X}:", offsetStart + (multiplier * 0x0010));
        }
      }

      log += fmt::format(" {:02X}", read(i, true));
      if (i == offsetStop)
      {
        log += "\n";
      }
    }

    return log;
   }
*/

  // Update Memory Map
/*
  void BaseBus::updateMemoryMap(uint16_t offset, uint8_t rows, bool clear)
  {
    if (clear)
    {
      memorymap.clear();
    }

    uint16_t offsetStart = offset & 0xFFF0;
    uint16_t offsetStop = (rows * (offset + 1)) | 0x000F;

    std::map<uint16_t, Bus::MEMORYMAP> memory = memoryDump(offsetStart, offsetStop);

    uint16_t multiplier = 0;
    for (auto& i : memory)
    {
      uint32_t offsetPos = (16 * (uint32_t)multiplier) + (uint32_t)offset;
      auto row = i.second;
      memorymap[row.Offset] = row;
    }
  }
*/

  std::map<uint16_t, BaseBus::MEMORYMAP> BaseBus::memoryDump(uint16_t offsetStart, uint16_t offsetStop)
  {
    std::map<uint16_t, BaseBus::MEMORYMAP> memory;
    uint16_t addr = offsetStart & 0xFFF0,
    multiplier = 0,
    offset = 0x00;
    
    while (addr <= (offsetStop & 0xFFF0))
    {
      offset = (offsetStart & 0xFFF0) + (multiplier * 0x0010);
      
      memory[multiplier] = {
        offset,             // Offset $
        cpuRead(addr++, true), // 0x00
        cpuRead(addr++, true), // 0x01
        cpuRead(addr++, true), // 0x02
        cpuRead(addr++, true), // 0x03
        cpuRead(addr++, true), // 0x04
        cpuRead(addr++, true), // 0x05
        cpuRead(addr++, true), // 0x06
        cpuRead(addr++, true), // 0x07
        cpuRead(addr++, true), // 0x08
        cpuRead(addr++, true), // 0x09
        cpuRead(addr++, true), // 0x0A
        cpuRead(addr++, true), // 0x0B
        cpuRead(addr++, true), // 0x0C
        cpuRead(addr++, true), // 0x0D
        cpuRead(addr++, true), // 0x0E
        cpuRead(addr, true)  // 0x0F
      };
      //++addr;
      ++multiplier;
    }
    return memory;
  }
};
