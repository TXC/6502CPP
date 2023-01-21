#pragma once

#include "MainTest.hpp"

//using namespace CPU;
//using namespace CPUTest;
namespace CPUTest
{
  class ProcessorTests
  {
  public:
    enum RegisterMode
    {
      Accumulator = 1,  // CMP Operation
      XRegister = 2,    // CPX Operation
      YRegister = 3     // CPY Operation
    };
  };  
};
