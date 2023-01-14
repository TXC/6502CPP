#pragma once

#include <array>
#include "Processor.hpp"

namespace CPU
{
  class Bus
  {
  public:
    Bus();
    ~Bus();

  public: // Devices on bus
    Processor cpu;

    // Fake RAM for this part of the series
    std::array<uint8_t, 64 * 1024> ram;


  public: // Bus Read & Write
    void reset();
    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr, bool bReadOnly = false);

  public: // DEBUG
    void dump(uint16_t offsetStart);
    void dump(uint16_t offsetStart, uint16_t offsetStop);
    std::string dumpRaw(uint16_t offsetStart, uint16_t offsetStop);
  };
};
