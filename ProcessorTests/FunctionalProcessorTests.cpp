#include "FunctionalProcessorTests.hpp"
#include <Types.hpp>


#include <catch2/catch_all.hpp>
#include <string>
//#include <sstream>

#include <fmt/format.h>
/*
#include <spdlog/spdlog.h>
#ifdef SPDLOG_FMT_EXTERNAL
#include <fmt/format.h>
#else
#include <spdlog/fmt/fmt.h>
#endif
*/

namespace CPUTest
{
  using namespace CPU;


/** 
 * Each Test Case in Klaus_Dormann's Functional Test Program. 
 * Note: Each test case also runs the tests before it. There wasn't a good way to just run each test case
 * If a test is failing find the first test that fails. The tests are dumb, they do not catch error traps correctly.
 * 
 * @see https://github.com/Klaus2m5/6502_65C02_functional_tests
 */
TEST_CASE("KlausDormann's Functional Test Program", "[.KlausDormann]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  Bus bus;

  FunctionalProcessorTests::PROGRAMDATA KdTestProgram = FunctionalProcessorTests::loadFunctionFile("6502_functional_test.bin");

  SECTION("Functional Test")
  {
    auto [accumulator, programCounter] = GENERATE( table<uint8_t, uint16_t>({
      {0x01, 0x0461}, // Load Data
      {0x02, 0x05aa}, // BNE Relative Addressing Test
      {0x03, 0x05f1}, // Partial test BNE & CMP, CPX, CPY immediate
      {0x04, 0x0625}, // Testing stack operations PHA PHP PLA PLP
      {0x05, 0x079f}, // Testing branch decisions BPL BMI BVC BVS BCC BCS BNE BEQ
      {0x06, 0x089b}, // Test PHA does not alter flags or accumulator but PLA does
      {0x07, 0x08cf}, // Partial pretest EOR #
      {0x08, 0x0919}, // PC modifying instructions except branches
                      // (NOP, JMP, JSR, RTS, BRK, RTI)
      {0x09, 0x096f}, // Jump absolute
      {0x0A, 0x09ab}, // Jump indirect
      {0x0B, 0x09e2}, // Jump subroutine & return from subroutine
      {0x0C, 0x0a14}, // Break and return from RTI
      {0x0D, 0x0aba}, // Test set and clear flags CLC CLI CLD CLV SEC SEI SED
      {0x0E, 0x0d80}, // Testing index register increment/decrement and transfer
                      // INX INY DEX DEY TAX TXA TAY TYA 
      {0x0F, 0x0e49}, // TSX sets NZ - TXS does not
      {0x10, 0x0f04}, // Testing index register load & store LDY LDX STY STX all
                      // addressing modes LDX / STX - zp,y / abs,y
      {0x11, 0x0f46}, // Indexed wraparound test (only zp should wrap)
      {0x12, 0x0ffd}, // LDY / STY - zp,x / abs,x
      {0x13, 0x103d}, // Indexed wraparound test (only zp should wrap)
      {0x14, 0x1333}, // LDX / STX - zp / abs / #
      {0x15, 0x162d}, // LDY / STY - zp / abs / #
      {0x16, 0x16de}, // Testing load / store accumulator LDA / STA all addressing
                      // modes LDA / STA - zp,x / abs,x
      {0x17, 0x17f9}, // LDA / STA - (zp),y / abs,y / (zp,x)
      {0x18, 0x189c}, // Indexed wraparound test (only zp should wrap)
      {0x19, 0x1b66}, // LDA / STA - zp / abs / #
      {0x1A, 0x1cba}, // testing bit test & compares BIT CPX CPY CMP all addressing
                      // modes BIT - zp / abs
      {0x1B, 0x1dc8}, // CPX - zp / abs / # 
      {0x1C, 0x1ed6}, // CPY - zp / abs / # 
      {0x1D, 0x22ba}, // CMP - zp / abs / # 
      {0x1E, 0x23fe}, // Testing shifts - ASL LSR ROL ROR, all addressing modes shifts
                      // - accumulator
      {0x1F, 0x257e}, // Shifts - zeropage
      {0x20, 0x2722}, // Shifts - absolute
      {0x21, 0x28a2}, // Shifts - zp indexed
      {0x22, 0x2a46}, // Shifts - abs indexed
      {0x23, 0x2af0}, // Testing memory increment/decrement - INC DEC, all addressing modes zeropage
      {0x24, 0x2baa}, // Absolute memory
      {0x25, 0x2c58}, // Zeropage indexed
      {0x26, 0x2d16}, // Memory indexed
      {0x27, 0x2f0e}, // Testing logical instructions - AND EOR ORA, all addressing modes AND
      {0x28, 0x3106}, // EOR
      {0x29, 0x32ff}, // OR
      {0x2A, 0x3364}, // Full binary add/subtract test iterates through all
                      // combinations of operands and carry input uses
                      // increments/decrements to predict result & result flags
      {0x2B, 0x3408}, // Binary Switch Test
      {0xF0, 0x3463}, // Decimal add/subtract test
                      // *** WARNING - tests documented behavior only! ***
                      // only valid BCD operands are tested, N V Z flags are ignored
                      // iterates through all valid combinations of operands and
                      // carry input uses increments/decrements to predict
                      // result & carry flag
    }));
    
    SECTION(
      fmt::format("TestCase: (0x{:02X}, 0x{:4X}) works", accumulator, programCounter)
    ) {
      bus.cpu.reset();
      bus.cpu.LoadProgram(0x400, KdTestProgram.program, KdTestProgram.size, 0x400);

      uint32_t numberOfCycles = 0;

      while(true)
      {
        bus.cpu.tick();
        ++numberOfCycles;

        if(hex(bus.cpu.getProgramCounter(), 4) == hex(programCounter, 4))
        {
          break;
        }

        if (numberOfCycles > 40037912) {
          FAIL("Maximum Number of Cycles Exceeded.");
        }
      }

      REQUIRE(hex(bus.cpu.getRegister(bus.cpu.AC), 2) == hex(accumulator, 2));
    }

  }
}

TEST_CASE("Klaus Dormann's Interrupt Test Program", "[.KlausDormann]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  Bus bus;

  FunctionalProcessorTests::PROGRAMDATA InterruptProgram = FunctionalProcessorTests::loadFunctionFile("6502_interrupt_test.bin");

  SECTION("Interrupt Test")
  {
    auto [programCounter, name] = GENERATE( table<uint16_t, std::string>({
      {0x04f9, "IRQ Tests"},
      {0x05b7, "BRK Tests"},
      {0x068d, "NMI Tests"},
      {0x06ec, "Disable Interrupt Tests"}
    }));

    SECTION(
      fmt::format("TestGroup: {} - 0x{:04X}", name, programCounter)
    ) {
      bus.cpu.reset();
      bus.cpu.LoadProgram(0x400, InterruptProgram.program, InterruptProgram.size, 0x400);
      uint32_t numberOfCycles = 0;
      uint8_t  previousInterruptWatchValue = 0,
              interruptWatch;

      while(true)
      {
        interruptWatch = bus.read(0xbffc);

        //This is used to simulate the edge triggering of an NMI. If we didn't do this we would get stuck in a loop forever
        if (interruptWatch != previousInterruptWatchValue)
        {
          previousInterruptWatchValue = interruptWatch;

          if ((interruptWatch & 2) != 0)
          {
            bus.cpu.TriggerNmi = true;
          }
        }

        if (bus.cpu.GetFlag(bus.cpu.I) == 0 && (interruptWatch & 1) != 0)
        {
          //bus.cpu.TriggerIRQ = true;
          bus.cpu.irq();
        }

        bus.cpu.tick();
        numberOfCycles++;

        if(bus.cpu.getProgramCounter() == programCounter)
        {
          break;
        }

        if (numberOfCycles > 100000) {
          FAIL("Maximum Number of Cycles Exceeded.");
        }
      }
    }
  }
}

/**
 * Ed Spittles Tests
 * 
 * @see https://github.com/BigEd/6502timing
 */
TEST_CASE("Ed Spittles Test Program", "[.EdSpittles]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  Bus bus;

  FunctionalProcessorTests::PROGRAMDATA CycleProgram = FunctionalProcessorTests::loadFunctionFile("6502_cycle_test.bin");
  std::vector<FunctionalProcessorTests::TESTDATA> CycleTestDataResults = FunctionalProcessorTests::loadCycleTestResults("cycle_test_data.csv", ",");

  SECTION("Cycle Test")
  {
    bus.cpu.LoadProgram(0x0000, CycleProgram.program, CycleProgram.size, 0x00);
    uint16_t numberOfLoops = 1;

    while(true)
    {
      if (numberOfLoops == 0xF9)
      {
        
      }

      bus.cpu.tick();

      fmt::print("Step: {} PC: {:04X}", numberOfLoops, bus.cpu.getProgramCounter());
      fmt::print("Step: {} Cycles: {}", numberOfLoops, bus.cpu.cycle_count);

      REQUIRE(hex(bus.cpu.getProgramCounter(), 4) == hex(CycleTestDataResults[numberOfLoops].PC, 4));
      REQUIRE(hex(bus.cpu.cycle_count, 4) == hex(CycleTestDataResults[numberOfLoops].CC, 4));

      numberOfLoops++;

      if(bus.cpu.getProgramCounter() == 0x1266)
      {
        break;
      }

      if (numberOfLoops > 500) {
        FAIL("Maximum Number of Cycles Exceeded.");
      }
    }

    REQUIRE(hex(bus.cpu.cycle_count, 4) == hex(0x0474, 4));
  }
}
}
