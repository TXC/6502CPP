#pragma once

#include "Bus.hpp"
#include "CPU.hpp"

#include <array>
#include <map>
#if defined SPDLOG_FMT_EXTERNAL
#include <fmt/format.h>
#include <fmt/ostream.h>
#else
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ostr.h>
#endif

namespace Processor
{
  class Logger;
  class MainBus : public Bus
  {
  public:
    MainBus();
    ~MainBus();

  public: // Devices on bus
    // The 6502 derived processor
    CPU cpu;

  public: // System Interface
    // Resets the system
    void reset() override;
    // Clocks the system - a single whole systme tick
    bool clock() override;
    // Check if the system is complete
    bool complete() override;
  };
};
