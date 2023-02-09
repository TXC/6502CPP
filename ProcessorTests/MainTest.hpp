#pragma once

#include <Processor/MainBus.hpp>
#include <Processor/Logger.hpp>

#include <string>

#include <fmt/format.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_section_info.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

namespace CPUTest
{
  //Bus bus;

  class MainTest
  {
  public:

    static void logTestCaseName(std::string name)
    {
      Processor::Logger::log()->info("");
      Processor::Logger::log()->info(" *** {: <32} ***", name);
      Processor::Logger::log()->info("");
    }

    static void logSectionName(std::string section)
    {
      Processor::Logger::log()->info("");
      Processor::Logger::log()->info(" *** SECTION {: <24} ***", section);
      Processor::Logger::log()->info("");
    }

  };
};
