#include "Bus.hpp"
#include "Types.hpp"
#include "Logger.hpp"
#include <iostream>
#if defined(SPDLOG_FMT_EXTERNAL)
#include <fmt/format.h>
#else
#include <spdlog/fmt/fmt.h>
#endif
#include <signal.h>

namespace CPU
{

  void handler(int sig)
  {
#if defined(DEBUG)
    std::cout << "DEBUG MODE ACTIVE!" << std::endl;
#endif
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
#if defined(DEBUG)
    std::cout << "Printing Backtrace:" << std::endl
              << Backtrace() << std::endl;
#endif
    exit(128 + sig);
  }


  Bus::Bus()
  {
    try
    {
      Logger log;

      signal(SIGABRT, handler);
      signal(SIGFPE, handler);
      signal(SIGILL, handler);
      signal(SIGINT, handler);
      signal(SIGSEGV, handler);
      signal(SIGTERM, handler);

      // Connect CPU to communication bus
      cpu.ConnectBus(this);
      
      // Clear RAM contents, just in case :P
      reset();
    }
    catch (const std::exception& e)
    {
      std::cout << "std::exception :" << e.what() << std::endl;
#if defined(DEBUG)
      print_exception(e);
#endif
    }
    catch (...)
    {
      std::cout << "catch_all exception" << std::endl;
      exit(1);
    }
  }

  Bus::~Bus()
  {
  }

  void Bus::reset()
  {
    for (auto& i : ram)
    {
      i = 0x00;
    }
  }

  void Bus::write(uint16_t addr, uint8_t data)
  {
    if (addr >= 0x0000 && addr <= 0xFFFF)
    {
      ram[addr] = data;
    }
  }

  uint8_t Bus::read(uint16_t addr, bool bReadOnly)
  {
    if (addr >= 0x0000 && addr <= 0xFFFF)
    {
      //Logger::log()->debug("RAM: ${:04X} = {:02X}", addr, ram[addr]);
      return ram[addr];
    }

    return 0x00;
  }

  void Bus::dump(uint16_t offset)
  {
#if defined(LOGMODE)
    Logger::log()->info("Actual ADDR: ${:04X}", offset);

    uint16_t offsetStart = offset & 0xFFF0;
    uint16_t offsetStop = offset | 0x000F;
    dump(offsetStart, offsetStop);
#endif
  }

  void Bus::dump(uint16_t offsetStart, uint16_t offsetStop)
  {
#if defined(LOGMODE)
    Logger::log()->info("MEMORY LOG FOR: ${:04X} - ${:04X}", offsetStart, offsetStop);
    Logger::log()->info(" ADDR 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");

    std::map<uint16_t, Bus::MEMORYMAP> memory = memoryDump(offsetStart, offsetStop);
    for (auto& i : memory)
    {
      Logger::log()->info("{}", i.second);
    }
#endif
  }

  /*
  std::string Bus::dumpRaw(uint16_t offsetStart, uint16_t offsetStop)
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

/*
  // Update Memory Map
  void Bus::updateMemoryMap(uint16_t offset, uint8_t rows, bool clear)
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

  std::map<uint16_t, Bus::MEMORYMAP> Bus::memoryDump(uint16_t offsetStart, uint16_t offsetStop)
  {
    std::map<uint16_t, Bus::MEMORYMAP> memory;
    uint16_t addr = offsetStart & 0xFFF0,
             multiplier = 0,
             offset = 0x00;

    while (addr <= (offsetStop & 0xFFF0))
    {
      offset = (offsetStart & 0xFFF0) + (multiplier * 0x0010);

      memory[multiplier] = {
        offset,             // Offset $
        read(addr++, true), // 0x00
        read(addr++, true), // 0x01
        read(addr++, true), // 0x02
        read(addr++, true), // 0x03
        read(addr++, true), // 0x04
        read(addr++, true), // 0x05
        read(addr++, true), // 0x06
        read(addr++, true), // 0x07
        read(addr++, true), // 0x08
        read(addr++, true), // 0x09
        read(addr++, true), // 0x0A
        read(addr++, true), // 0x0B
        read(addr++, true), // 0x0C
        read(addr++, true), // 0x0D
        read(addr++, true), // 0x0E
        read(addr, true)  // 0x0F
      };
      //++addr;
      ++multiplier;
    }
    return memory;
  }
}
