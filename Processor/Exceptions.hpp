#pragma once

#include <string>
#include <stdexcept>

namespace CPU
{
  class JammedCPU : public std::exception
  {
    private:
      char * message;

    public:
      JammedCPU(char * msg): message(msg) {}
      char * what () {
        return message;
      }
  };
}