#include "Bus.hpp"
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
  Bus::Bus() : BaseBus()
  {
    cpu.connectBus(this);
  }
  
  Bus::~Bus()
  {
  }

/*
  void Bus::cpuWrite(uint16_t addr, uint8_t data)
  {
    if (addr >= 0x0000 && addr <= 0xFFFF)
    {
      ram[addr] = data;
    }
  }
*/

/*
  uint8_t Bus::cpuRead(uint16_t addr, bool bReadOnly)
  {
    if (addr >= 0x0000 && addr <= 0xFFFF)
    {
      return ram[addr];
    }
    
    return 0x00;
  }
*/

  void Bus::reset()
  {
    BaseBus::reset();
    cpu.reset();
  }

  void Bus::clock()
  {
    BaseBus::clock();
    cpu.clock();
    
    nSystemClockCounter++;
  }

  bool Bus::complete()
  {
    return cpu.complete() & BaseBus::complete();
  }
}
