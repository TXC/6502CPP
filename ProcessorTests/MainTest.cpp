#include "MainTest.hpp"
#include "Types.hpp"
/*
#include <iostream>
#include <signal.h>
#include <catch2/catch_session.hpp>

namespace CPUTest
{
}

void handler(int sig)
{
  std::cout << "DEBUG MODE ACTIVE!" << std::endl;

  switch(sig)
  {
    case SIGABRT:
      std::cout << "Abort Detected." << std::endl;
      break;
    case SIGFPE:
      std::cout << "Floating Point Exception Detected." << std::endl;
      break;
    case SIGILL:
      std::cout << "Illegal Instruction Detected." << std::endl;
      break;
    case SIGINT:
      std::cout << "Interrupt Detected." << std::endl;
      break;
    case SIGSEGV:
      std::cout << "Segmention Fault Detected." << std::endl;
      break;
    case SIGTERM:
      std::cout << "Termination Detected." << std::endl;
      break;
    default:
      std::cout << "Unknown Fault Detected." << std::endl;
  }
#if defined DEBUG
  std::cout << "Printing Backtrace:" << std::endl
            << CPU::Backtrace() << std::endl;
#endif
  exit(128 + sig);
}

int main( int argc, char* argv[])
{
    signal(SIGABRT, handler);
    signal(SIGFPE, handler);
    signal(SIGILL, handler);
    signal(SIGINT, handler);
    signal(SIGSEGV, handler);
    signal(SIGTERM, handler);


    std::cout << "HEJ HEJ" << std::endl;

    int result = Catch::Session().run( argc, argv );

    return result;
}
*/
