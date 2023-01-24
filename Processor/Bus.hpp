#pragma once

#include "Processor.hpp"

#include <array>
#include <map>
#if defined(SPDLOG_FMT_EXTERNAL)
#include <fmt/format.h>
#include <fmt/ostream.h>
#else
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ostr.h>
#endif

namespace CPU
{
  class Logger;
  class Bus
  {
  public:
    Bus();
    ~Bus();

  public: // Devices on bus
    Processor cpu;

    // Fake RAM for this part of the series
    std::array<uint8_t, 64 * 1024> ram;

    struct MEMORYMAP
    {
      uint16_t  Offset;
      uint8_t   Pos00;
      uint8_t   Pos01;
      uint8_t   Pos02;
      uint8_t   Pos03;
      uint8_t   Pos04;
      uint8_t   Pos05;
      uint8_t   Pos06;
      uint8_t   Pos07;
      uint8_t   Pos08;
      uint8_t   Pos09;
      uint8_t   Pos0A;
      uint8_t   Pos0B;
      uint8_t   Pos0C;
      uint8_t   Pos0D;
      uint8_t   Pos0E;
      uint8_t   Pos0F;

      friend std::ostream &operator<<(std::ostream &os, const MEMORYMAP& obj)
      {
        fmt::format_to(
          std::ostream_iterator<char>(os),
          "${:04X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X}",
          obj.Offset,
          obj.Pos00, obj.Pos01, obj.Pos02, obj.Pos03,
          obj.Pos04, obj.Pos05, obj.Pos06, obj.Pos07,
          obj.Pos08, obj.Pos09, obj.Pos0A, obj.Pos0B,
          obj.Pos0C, obj.Pos0D, obj.Pos0E, obj.Pos0F
        );
        return os;
      }
    };

  private:
    std::map<uint16_t, MEMORYMAP> memorymap = {};

  public: // Bus Read & Write
    void reset();
    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr, bool bReadOnly = false);

  public: // DEBUG
    std::map<uint16_t, MEMORYMAP> memoryDump(uint16_t offsetStart, uint16_t offsetStop);
    //void updateMemoryMap(uint16_t offset = 0x0000, uint8_t rows = 0xFF, bool clear = true);
    void dump(uint16_t offsetStart);
    void dump(uint16_t offsetStart, uint16_t offsetStop);
  };
};
