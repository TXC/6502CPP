#pragma once

#include "BaseBus.hpp"
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
  class Bus : public BaseBus
  {
  public:
    Bus();
    ~Bus();

  public: // Devices on bus
    // The 6502 derived processor
    CPU cpu;

  public: // System Interface
    // Resets the system
    void reset() override;
    // Clocks the system - a single whole systme tick
    void clock() override;
    // Check if the system is complete
    bool complete() override;
  };
};
