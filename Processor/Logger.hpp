#pragma once

#include "Formatters.hpp"
#include <spdlog/spdlog.h>

namespace CPU
{
  static std::shared_ptr<spdlog::logger> logger = nullptr;
  class Logger
  {
  public:
    Logger();
    ~Logger();

#ifdef LOGMODE
    //static FILE* logfile = nullptr;
    //static std::ofstream logfile;
#endif
  private:
    static void createLogger();

  public:
    static std::shared_ptr<spdlog::logger> log();
    static std::shared_ptr<spdlog::logger> log(std::string text);
  };
};
