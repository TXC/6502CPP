#pragma once

#include <Bus.hpp>
#include <Logger.hpp>

#include <string>

#include <fmt/format.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_section_info.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

namespace CPUTest
{
  using namespace CPU;
  //Bus bus;

  class MainTest
  {
  public:

    static void logTestCaseName(std::string name)
    {
      CPU::Logger::log()->info("");
      CPU::Logger::log()->info(" *** {: <32} ***", name);
      CPU::Logger::log()->info("");
    }

    static void logSectionName(std::string section)
    {
      CPU::Logger::log()->info("");
      CPU::Logger::log()->info(" *** SECTION {: <24} ***", section);
      CPU::Logger::log()->info("");
    }

  };
};
