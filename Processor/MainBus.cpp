#include "MainBus.hpp"
#include "Types.hpp"
#include "Logger.hpp"

#include <iostream>
#if defined SPDLOG_FMT_EXTERNAL
#include <fmt/format.h>
#else
#include <spdlog/fmt/fmt.h>
#endif
#include <signal.h>

namespace Processor
{
  MainBus::MainBus() : Bus()
  {
    cpu.connectBus(this);
  }
  
  MainBus::~MainBus()
  {
  }

/*
  void MainBus::cpuWrite(uint16_t addr, uint8_t data)
  {
    if (addr >= 0x0000 && addr <= 0xFFFF)
    {
      ram[addr] = data;
    }
  }
*/

/*
  uint8_t MainBus::cpuRead(uint16_t addr, bool bReadOnly)
  {
    if (addr >= 0x0000 && addr <= 0xFFFF)
    {
      return ram[addr];
    }
    
    return 0x00;
  }
*/

  void MainBus::reset()
  {
    Bus::reset();
    cpu.reset();
  }

  // 6502 CPU doesn't implement audio
  bool MainBus::clock()
  {
    Bus::clock();
    cpu.clock();
    
    nSystemClockCounter++;

    return false;
  }

  bool MainBus::complete()
  {
    return cpu.complete() & Bus::complete();
  }
}
