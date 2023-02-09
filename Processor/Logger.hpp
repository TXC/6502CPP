#pragma once

#include "Formatters.hpp"

#include <memory>
#include <cstdlib>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/pattern_formatter.h>


#ifndef LOGFILE
#define LOGFILE Processor.log
#endif

#define _CPUSTRINGIZE(x) #x
#define CPUSTRINGIZE(x) _CPUSTRINGIZE(x)

namespace Processor
{
  static std::shared_ptr<spdlog::logger> logger = nullptr;
  class Logger
  {
  //public:
  //  Logger();
  //  ~Logger();

  private:
    static void createLogger()
    {
      try
      {
#if defined LOGMODE
#if _WIN32
        std::filesystem::path filename = std::string(std::getenv("HOMEPATH"));
#else
        std::filesystem::path filename = std::string(std::getenv("HOME"));
#endif
        filename /= std::string(CPUSTRINGIZE(LOGFILE));
        /**
         * Thread Safe
         */
        logger = spdlog::basic_logger_mt("CPU6502", filename.string());

        /**
         * Single Threaded
         */
        //logger = spdlog::basic_logger_st("CPU6502", filename.string());
#else
        logger = spdlog::create<spdlog::sinks::null_sink_st>("CPU6502");
#endif
        logger->set_pattern("%Y-%m-%d %T.%e [%=10l] %v");
#if defined DEBUG
        // Set global log level to debug
        logger->set_level(spdlog::level::debug);
#endif
      }
      catch (const spdlog::spdlog_ex &ex)
      {
        std::cout << "Log init failed: " << ex.what() << std::endl;
      }
    }

  public:
    static std::shared_ptr<spdlog::logger> log()
    {
      if (logger == nullptr)
      {
        createLogger();
      }
      return logger;
    }
/*
  public:
    template<typename... Args>
    static void trace(spdlog::format_string_t<Args...> fmt, Args &&... args)
    {
      if (logger == nullptr)
      {
        createLogger();
      }
      logger->log(spdlog::level::trace, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void debug(spdlog::format_string_t<Args...> fmt, Args &&... args)
    {
      if (logger == nullptr)
      {
        createLogger();
      }
      logger->log(spdlog::level::debug, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void info(spdlog::format_string_t<Args...> fmt, Args &&... args)
    {
      if (logger == nullptr)
      {
        createLogger();
      }
      logger->log(spdlog::level::info, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void warn(spdlog::format_string_t<Args...> fmt, Args &&... args)
    {
      if (logger == nullptr)
      {
        createLogger();
      }
      logger->log(spdlog::level::warn, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void error(spdlog::format_string_t<Args...> fmt, Args &&... args)
    {
      if (logger == nullptr)
      {
        createLogger();
      }
      logger->log(spdlog::level::err, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void critical(spdlog::format_string_t<Args...> fmt, Args &&... args)
    {
      if (logger == nullptr)
      {
        createLogger();
      }
      logger->log(spdlog::level::critical, fmt, std::forward<Args>(args)...);
    }
*/
  };
};
