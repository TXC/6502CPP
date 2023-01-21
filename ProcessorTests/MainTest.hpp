#pragma once

#include <Bus.hpp>
#include <Logger.hpp>

#include <string>

#include <fmt/format.h>
//#include <spdlog/spdlog.h>
/*
#ifdef SPDLOG_FMT_EXTERNAL
#include <fmt/format.h>
//#include <fmt/ostream.h>
#else
#include <spdlog/fmt/fmt.h>
//#include <spdlog/fmt/ostr.h>
#endif
*/


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

  };
};
