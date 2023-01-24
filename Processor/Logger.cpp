#include "Logger.hpp"
#include "Formatters.hpp"

//#include <cstdio>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#ifndef LOGFILE
#define LOGFILE Processor.log
#endif

#define _STRINGIZE(x) #x
#define STRINGIZE(x) _STRINGIZE(x)

namespace CPU
{
  Logger::Logger()
  {
    //createLogger();
  }
  Logger::~Logger()
  {

  }

  void Logger::createLogger()
  {
    std::filesystem::path filename = std::filesystem::current_path() / std::string(STRINGIZE(LOGFILE));
    try 
    {
      /**
       * Thread Safe
       */
      logger = spdlog::basic_logger_mt("CPU6502", filename.string());

      /**
       * Single Threaded
       */
      //logger = spdlog::basic_logger_st("CPU6502", filename.string());

      //std::cout << "Logging to: " << filename.string() << std::endl;
      logger->set_pattern("[%Y-%m-%d %T.%e] [%=10l] %v");
#if defined(DEBUG)
      // Set global log level to debug
      logger->set_level(spdlog::level::debug);
#endif
    }
    catch (const spdlog::spdlog_ex &ex)
    {
      std::cout << "Log init failed: " << ex.what() << std::endl;
      std::fstream fs(filename, std::ios::in);
      fs << "Log init failed: " << ex.what() << std::endl;
      fs.flush();
      fs.close();
    }
  }

  std::shared_ptr<spdlog::logger> Logger::log()
  {
    if (logger == nullptr)
    {
      createLogger();
    }
    return logger;
  }
  std::shared_ptr<spdlog::logger> Logger::log(std::string text)
  {
    if (logger == nullptr)
    {
      std::cout << "No logger!" << std::endl;
      createLogger();
    }
    std::cout << "[DEPRECATED] Logger::log(std::string text) " << std::endl;
    logger->info(text);
    return logger;
  }
};
