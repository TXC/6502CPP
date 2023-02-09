#pragma once

#include <string>
#include <stdexcept>

namespace Processor
{
  class JammedCPU : public std::exception
  {
    private:
      std::string message;

    public:
      JammedCPU(char * msg): message(msg) {}
      JammedCPU(const std::string msg): message(msg) {}
      const char * what () const _NOEXCEPT {
        return message.c_str();
      }
  };
  class IllegalInstruction : public std::exception
  {
    private:
      std::string message;

    public:
      IllegalInstruction(char * msg): message(msg) {}
      IllegalInstruction(const std::string msg): message(msg) {}
      const char * what () const _NOEXCEPT {
        return message.c_str();
      }
  };
}
