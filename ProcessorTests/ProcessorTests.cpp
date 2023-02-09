#include "ProcessorTests.hpp"

#include <Processor/Types.hpp>

#include <iostream>
#include <catch2/catch_all.hpp>
#include <string>
#include <fmt/format.h>


namespace CPUTest
{
  using namespace Processor;

TEST_CASE("Show instructions loaded", "[.debug][.print]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  bus.cpu.executioner.printInstructions();

  SUCCEED("1 + 2 = 1");
}

TEST_CASE("Initialization Tests", "[init]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  SECTION("Processor Status Flags Initialized Correctly")
  {
    // C = Carry Bit
    // Z = Zero
    // I = Disable Interrupts
    // D = Decimal Mode (unused in this implementation)
    // B = Break
    // U = Unused
    // V = Overflow
    // N = Negative
    REQUIRE(bus.cpu.getFlag(bus.cpu.C) == 0);
    REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == 0);
    REQUIRE(bus.cpu.getFlag(bus.cpu.I) == 0);
    REQUIRE(bus.cpu.getFlag(bus.cpu.D) == 0);
    REQUIRE(bus.cpu.getFlag(bus.cpu.B) == 0);
    REQUIRE(bus.cpu.getFlag(bus.cpu.U) == 0);
    REQUIRE(bus.cpu.getFlag(bus.cpu.V) == 0);
    REQUIRE(bus.cpu.getFlag(bus.cpu.N) == 0);
  }

  SECTION("Processor Registers Initialized Correctly")
  {
    REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(0x00, 2));
    REQUIRE(hex(bus.cpu.getRegisterX(), 2) == hex(0x00, 2));
    REQUIRE(hex(bus.cpu.getRegisterY(), 2) == hex(0x00, 2));
    REQUIRE(hex(bus.cpu.getOpCode(), 2) == hex(0x00, 2));
    REQUIRE(hex(bus.cpu.getProgramCounter(), 4) == hex(0x0000, 4));
  }

  SECTION("Stack Pointer Initialized Correctly")
  {
    REQUIRE(hex(bus.cpu.getRegisterSP(), 2) == hex(0x00, 2));
  }

  SECTION("ProgramCounter Correct When Program Loaded")
  {
    uint8_t program[] = {0x01};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x01);
    REQUIRE(hex(bus.cpu.getProgramCounter(), 2) == hex(0x01, 2));
  }

#ifndef ILLEGAL
  SECTION("Throws Exception When OpCode Is Invalid")
  {
    uint8_t program[] = {0xFF};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);
    REQUIRE_THROWS(bus.cpu.clock());
  }
#endif

  SECTION("Stack Pointer Initializes To Default Value After Reset")
  {
    bus.cpu.reset();

    REQUIRE(hex(bus.cpu.getRegisterSP(), 2) == hex(0xFD, 2));
  }

  SECTION("Processor Status Flags Initializes To Default Value After Reset")
  {
    bus.cpu.reset();

    REQUIRE(bus.cpu.getFlag(CPU::FLAGS6502::C) == 0);
    REQUIRE(bus.cpu.getFlag(CPU::FLAGS6502::Z) == 0);
    REQUIRE(bus.cpu.getFlag(CPU::FLAGS6502::I) == 0);
    REQUIRE(bus.cpu.getFlag(CPU::FLAGS6502::D) == 0);
    REQUIRE(bus.cpu.getFlag(CPU::FLAGS6502::B) == 1);
    REQUIRE(bus.cpu.getFlag(CPU::FLAGS6502::U) == 1); // Always set
    REQUIRE(bus.cpu.getFlag(CPU::FLAGS6502::V) == 0);
    REQUIRE(bus.cpu.getFlag(CPU::FLAGS6502::N) == 0);
  }
}

#pragma region OPCode
TEST_CASE("ADC - Add with Carry Tests", "[opcode][adc]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;
  uint8_t operation = 0x69;

  SECTION("ADC Accumulator Correct When Not In BDC Mode")
  {

    auto [accumulatorInitialValue, amountToAdd, CarryFlagSet, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, bool, uint8_t>({
        {0x00, 0x00, false, 0x00},
        {0x00, 0x01, false, 0x01},
        {0x01, 0x02, false, 0x03},
        {0xFF, 0x01, false, 0x00},
        {0xFE, 0x01, false, 0xFF},
        {0xFF, 0x00, false, 0xFF},
        {0x00, 0x00, true, 0x01},
        {0x00, 0x01, true, 0x02},
        {0x01, 0x02, true, 0x04},
        {0xFE, 0x01, true, 0x00},
        {0xFD, 0x01, true, 0xFF},
        {0xFE, 0x00, true, 0xFF},
        {0xFF, 0xFF, true, 0xFF}
      }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorInitialValue, amountToAdd, CarryFlagSet, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      if (CarryFlagSet)
      { 
        uint8_t program[] = {0x38, 0xA9, accumulatorInitialValue, 0x69, amountToAdd};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.loadProgram(0x0000, program, n, 0x00);
        bus.cpu.clock();
      }
      else
      {
        uint8_t program[] = {0xA9, accumulatorInitialValue, 0x69, amountToAdd};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.loadProgram(0x0000, program, n, 0x00);
      }

      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(accumulatorInitialValue, 2));

      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(expectedValue, 2));
    }
  }

  SECTION("ADC Accumulator Correct When In BDC Mode")
  {
    auto [accumulatorInitialValue, amountToAdd, CarryFlagSet, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, bool, uint8_t>({
        {0x99, 0x99, false, 0x98},
        {0x99, 0x99, true, 0x99},
        {0x90, 0x99, false, 0x89}
      }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorInitialValue, amountToAdd, CarryFlagSet, expectedValue)
    ) {

      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      if (CarryFlagSet)
      { 
        uint8_t program[] = {0x38, 0xF8, 0xA9, accumulatorInitialValue, 0x69, amountToAdd};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.loadProgram(0x0000, program, n, 0x00);
        bus.cpu.clock();
      }
      else
      {
        uint8_t program[] = {0xF8, 0xA9, accumulatorInitialValue, 0x69, amountToAdd};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.loadProgram(0x0000, program, n, 0x00);
      }

      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(accumulatorInitialValue, 2));
      
      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(expectedValue, 2));
    }
  }

  SECTION("ADC Carry Correct When Not In BDC Mode")
  {
    MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

    auto [accumulatorInitialValue, amountToAdd, CarryFlagSet, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, bool, uint8_t>({
        {0xFE, 0x01, false, 0},
        {0xFE, 0x01, true, 1},
        {0xFD, 0x01, true, 0},
        {0xFF, 0x01, false, 1},
        {0xFF, 0x01, true, 1}
      }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorInitialValue, amountToAdd, CarryFlagSet, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      if (CarryFlagSet)
      { 
        uint8_t program[] = {0x38, 0xA9, accumulatorInitialValue, 0x69, amountToAdd};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.loadProgram(0x0000, program, n, 0x00);
        bus.cpu.clock();
      }
      else
      {
        uint8_t program[] = {0xA9, accumulatorInitialValue, 0x69, amountToAdd};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.loadProgram(0x0000, program, n, 0x00);
      }

      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(accumulatorInitialValue, 2));
      
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.C) == expectedValue);
    }
  }

  SECTION("ADC Carry Correct When In BDC Mode")
  {
    auto [accumulatorInitialValue, amountToAdd, CarryFlagSet, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, bool, uint8_t>({
        {0x62, 0x01, false, 0},
        {0x62, 0x01, true, 0},
        {0x63, 0x01, false, 0},
        {0x63, 0x01, true, 0}
      }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorInitialValue, amountToAdd, CarryFlagSet, expectedValue)
    ) {

      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      uint8_t program[] = {0xF8, 0xA9, accumulatorInitialValue, 0x69, amountToAdd};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(accumulatorInitialValue, 2));
      
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.C) == expectedValue);
    }
  }

  SECTION("ADC Zero Flag Correct When Not In BDC Mode")
  {
    auto [accumulatorInitialValue, amountToAdd, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t>({
        {0x00, 0x00, 1},
        {0xFF, 0x01, 1},
        {0x00, 0x01, 0},
        {0x01, 0x00, 0}
      }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorInitialValue, amountToAdd, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      uint8_t program[] = {0xA9, accumulatorInitialValue, 0x69, amountToAdd};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(accumulatorInitialValue, 2));
      
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedValue);
    }
  }

  SECTION("ADC Negative Flag Correct")
  {
    auto [accumulatorInitialValue, amountToAdd, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t>({
        {0x7E, 0x01, 0},
        {0x01, 0x7E, 0},
        {0x01, 0x7F, 1},
        {0x7F, 0x01, 1},
        {0x01, 0xFE, 1},
        {0xFE, 0x01, 1},
        {0x01, 0xFF, 0},
        {0xFF, 0x01, 0},
      }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorInitialValue, amountToAdd, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      uint8_t program[] = {0xA9, accumulatorInitialValue, 0x69, amountToAdd};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(accumulatorInitialValue, 2));
      
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedValue);
    }
  }

  SECTION("ADC Overflow Flag Correct")
  {
    auto [accumulatorInitialValue, amountToAdd, CarryFlagSet, expectedValue] = GENERATE( table<uint8_t, uint8_t, bool, uint8_t>({
      {0x00, 0x7F, false, 0},
      {0x00, 0x80, false, 0},
      {0x01, 0x7F, false, 1},
      {0x01, 0x80, false, 0},
      {0x7F, 0x01, false, 1},
      {0x7F, 0x7F, false, 1},
      {0x80, 0x7F, false, 0},
      {0x80, 0x80, false, 1},
      {0x80, 0x81, false, 1},
      {0x80, 0xFF, false, 1},
      {0xFF, 0x00, false, 0},
      {0xFF, 0x01, false, 0},
      {0xFF, 0x7F, false, 0},
      {0xFF, 0x80, false, 1},
      {0xFF, 0xFF, false, 0},
      {0x00, 0x7F, true, 1},
      {0x00, 0x80, true, 0},
      {0x01, 0x7F, true, 1},
      {0x01, 0x80, true, 0},
      {0x7F, 0x01, true, 1},
      {0x7F, 0x7F, true, 1},
      {0x80, 0x7F, true, 0},
      {0x80, 0x80, true, 1},
      {0x80, 0x81, true, 1},
      {0x80, 0xFF, true, 0},
      {0xFF, 0x00, true, 0},
      {0xFF, 0x01, true, 0},
      {0xFF, 0x7F, true, 0},
      {0xFF, 0x80, true, 0},
      {0xFF, 0xFF, true, 0}
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorInitialValue, amountToAdd, CarryFlagSet, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      if (CarryFlagSet)
      { 
        uint8_t program[] = {0x38, 0xA9, accumulatorInitialValue, 0x69, amountToAdd};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.loadProgram(0x0000, program, n, 0x00);
        bus.cpu.clock();
      }
      else
      {
        uint8_t program[] = {0xA9, accumulatorInitialValue, 0x69, amountToAdd};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.loadProgram(0x0000, program, n, 0x00);
      }

      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(accumulatorInitialValue, 2));
      
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.V) == expectedValue);
    }
  }
}

TEST_CASE("AND - Compare Memory with Accumulator", "[opcode][and]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x29;

  SECTION("AND Accumulator Correct")
  {
    auto [accumulatorInitialValue, amountToAdd, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t>({
      {0x00, 0x00, 0x00},
      {0xFF, 0xFF, 0xFF},
      {0xFF, 0xFE, 0xFE},
      {0xAA, 0x55, 0x00}
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorInitialValue, amountToAdd, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      uint8_t program[] = {0xA9, accumulatorInitialValue, 0x29, amountToAdd};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(accumulatorInitialValue, 2));
      
      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(expectedValue, 2));
    }
  }
}

TEST_CASE("ASL - Arithmetic Shift Left", "[opcode][asl]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;
  uint8_t operation = 0x0A;

  SECTION("ASL - Arithmetic Shift Left")
  {
    auto [sectionOperation, valueToShift, expectedValue, expectedLocation] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
        {0x0A, 0x6D, 0xDA, 0x0000},
        {0x0A, 0x6C, 0xD8, 0x0000},
        {0x06, 0x6D, 0xDA, 0x0001},
        {0x16, 0x6D, 0xDA, 0x0001},
        {0x0E, 0x6D, 0xDA, 0x0001},
        {0x1E, 0x6D, 0xDA, 0x0001},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:04X}) works",
      bus.cpu.executioner.getOperation(sectionOperation), valueToShift, expectedValue, expectedLocation)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      uint8_t program[] = {0xA9, valueToShift, sectionOperation, expectedLocation};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      if (sectionOperation == 0x0A)
      {
        REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(expectedValue, 2));
      }
      else
      {
        REQUIRE(hex(bus.cpuRead(expectedLocation, false), 2) == hex(expectedValue, 2));
      }
    }
  }

  SECTION("ASL Carry Set Correctly")
  {
    auto [valueToShift, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t>({
        {0x7F, 0},
        {0x80, 1},
        {0xFF, 1},
        {0x00, 0},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), valueToShift, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      uint8_t program[] = {0xA9, valueToShift, 0x0A};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.C) == expectedValue);
    }
  }

  SECTION("ASL Negative Set Correctly")
  {
    auto [valueToShift, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t>({
        {0x3F, 0},
        {0x40, 1},
        {0x7F, 1},
        {0x80, 0},
        {0x00, 0},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), valueToShift, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      uint8_t program[] = {0xA9, valueToShift, 0x0A};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedValue);
    }
  }

  SECTION("ASL Zero Set Correctly")
  {
    auto [valueToShift, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t>({
        {0x7F, 0},
        {0x80, 1},
        {0x00, 1},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), valueToShift, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      uint8_t program[] = {0xA9, valueToShift, 0x0A};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedValue);
    }
  }
}

TEST_CASE("BCC - Branch On Carry Clear", "[opcode][bcc]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x90;

  SECTION("BCC Program Counter Correct")
  {
    auto [programCounterInitalValue, programOffset, expectedValue] = GENERATE( table<uint16_t, uint8_t, uint16_t>({
      {0x00, 0x01, 0x03},
      {0x80, 0x80, 0x02},
      {0x00, 0x03, 0x05},
      {0x00, 0xFD, 0xFFFF},
      {0x7D, 0x80, 0xFFFF},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:04X}, 0x{:02X}, 0x{:04X}) works",
      bus.cpu.executioner.getOperation(operation), programCounterInitalValue, programOffset, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0x90, programOffset};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(programCounterInitalValue, program, n, programCounterInitalValue);
      REQUIRE(hex(bus.cpu.getProgramCounter(), 4) == hex(programCounterInitalValue, 4));
      bus.cpu.dumpRam(0x0000);

      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getProgramCounter(), 4) == hex(expectedValue, 4));
    }
  }
}

TEST_CASE("BCS - Branch on Carry Set", "[opcode][bcs]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0xB0;

  SECTION("BCS Program Counter Correct")
  {
    auto [programCounterInitalValue, programOffset, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, uint16_t>({
        {0x00, 0x01, 0x04},
        {0x80, 0x80, 0x03},
        {0x00, 0xFC, 0xFFFF},
        {0x7C, 0x80, 0xFFFF},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:04X}) works",
      bus.cpu.executioner.getOperation(operation), programCounterInitalValue, programOffset, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0x38, 0xB0, programOffset};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(programCounterInitalValue, program, n, programCounterInitalValue);
      REQUIRE(bus.cpu.getProgramCounter() == programCounterInitalValue);

      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getProgramCounter(), 4) == hex(expectedValue, 4));
    }
  }
}

TEST_CASE("BEQ - Branch on Zero Set", "[opcode][beq]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0xF0;

  SECTION("BEQ Program Counter Correct")
  {
    MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

    auto [programCounterInitalValue, programOffset, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, uint16_t>({
        {0x00, 0x01, 0x05},
        {0x80, 0x80, 0x04},
        {0x00, 0xFB, 0xFFFF},
        {0x7B, 0x80, 0xFFFF},
        {0x02, 0xFE, 0x04},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), programCounterInitalValue, programOffset, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, 0x00, 0xF0, programOffset};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(programCounterInitalValue, program, n, programCounterInitalValue);
      REQUIRE(bus.cpu.getProgramCounter() == programCounterInitalValue);

      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getProgramCounter(), 4) == hex(expectedValue, 4));
    }
  }
}

TEST_CASE("BIT - Compare Memory with Accumulator", "[opcode][bit]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  SECTION("BIT Negative Set When Comparison Is Negative Number")
  {
    auto [operation, accumulatorValue, valueToTest, expectedResult] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
        {0x24, 0x7f, 0x7F, 0}, // BIT Zero Page
        {0x24, 0x80, 0x7F, 0}, // BIT Zero Page
        {0x24, 0x7F, 0x80, 1}, // BIT Zero Page
        {0x24, 0x80, 0xFF, 1}, // BIT Zero Page
        {0x24, 0xFF, 0x80, 1}, // BIT Zero Page
        {0x2C, 0x7F, 0x7F, 0}, // BIT Absolute
        {0x2C, 0x80, 0x7F, 0}, // BIT Absolute
        {0x2C, 0x7F, 0x80, 1}, // BIT Absolute
        {0x2C, 0x80, 0xFF, 1}, // BIT Absolute
        {0x2C, 0xFF, 0x80, 1}, // BIT Absolute
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, valueToTest, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorValue, operation, 0x06, 0x00, 0x00, valueToTest};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x00, program, n, 0x00);
      REQUIRE(bus.cpu.getProgramCounter() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedResult);
    }
  }

  SECTION("BIT Overflow Set By Bit Six")
  {
    auto [operation, accumulatorValue, valueToTest, expectedResult] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
        {0x24, 0x3F, 0x3F, 0}, // BIT Zero Page
        {0x24, 0x3F, 0x40, 1}, // BIT Zero Page
        {0x24, 0x40, 0x3F, 0}, // BIT Zero Page
        {0x24, 0x40, 0x7F, 1}, // BIT Zero Page
        {0x24, 0x7F, 0x40, 1}, // BIT Zero Page
        {0x24, 0x7F, 0x80, 0}, // BIT Zero Page
        {0x24, 0x80, 0x7F, 1}, // BIT Zero Page
        {0x24, 0xC0, 0xDF, 1}, // BIT Zero Page
        {0x24, 0xDF, 0xC0, 1}, // BIT Zero Page
        {0x24, 0x3F, 0x3F, 0}, // BIT Zero Page
        {0x24, 0xC0, 0xFF, 1}, // BIT Zero Page
        {0x24, 0xFF, 0xC0, 1}, // BIT Zero Page
        {0x24, 0x40, 0xFF, 1}, // BIT Zero Page
        {0x24, 0xFF, 0x40, 1}, // BIT Zero Page
        {0x24, 0xC0, 0x7F, 1}, // BIT Zero Page
        {0x24, 0x7F, 0xC0, 1}, // BIT Zero Page
        {0x2C, 0x3F, 0x3F, 0}, // BIT Absolute
        {0x2C, 0x3F, 0x40, 1}, // BIT Absolute
        {0x2C, 0x40, 0x3F, 0}, // BIT Absolute
        {0x2C, 0x40, 0x7F, 1}, // BIT Absolute
        {0x2C, 0x7F, 0x40, 1}, // BIT Absolute
        {0x2C, 0x7F, 0x80, 0}, // BIT Absolute
        {0x2C, 0x80, 0x7F, 1}, // BIT Absolute
        {0x2C, 0xC0, 0xDF, 1}, // BIT Absolute
        {0x2C, 0xDF, 0xC0, 1}, // BIT Absolute
        {0x2C, 0x3F, 0x3F, 0}, // BIT Absolute
        {0x2C, 0xC0, 0xFF, 1}, // BIT Absolute
        {0x2C, 0xFF, 0xC0, 1}, // BIT Absolute
        {0x2C, 0x40, 0xFF, 1}, // BIT Absolute
        {0x2C, 0xFF, 0x40, 1}, // BIT Absolute
        {0x2C, 0xC0, 0x7F, 1}, // BIT Absolute
        {0x2C, 0x7F, 0xC0, 1}, // BIT Absolute
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, valueToTest, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorValue, operation, 0x06, 0x00, 0x00, valueToTest};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x00, program, n, 0x00);
      REQUIRE(bus.cpu.getProgramCounter() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.V) == expectedResult);
    }
  }

  SECTION("BIT Zero Set When Comparison Is Zero")
  {
    auto [operation, accumulatorValue, valueToTest, expectedResult] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
        {0x24, 0x00, 0x00, 1}, // BIT Zero Page
        {0x24, 0xFF, 0xFF, 0}, // BIT Zero Page
        {0x24, 0xAA, 0x55, 1}, // BIT Zero Page
        {0x24, 0x55, 0xAA, 1}, // BIT Zero Page
        {0x2C, 0x00, 0x00, 1}, // BIT Absolute
        {0x2C, 0xFF, 0xFF, 0}, // BIT Absolute
        {0x2C, 0xAA, 0x55, 1}, // BIT Absolute
        {0x2C, 0x55, 0xAA, 1}, // BIT Absolute
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, valueToTest, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorValue, operation, 0x06, 0x00, 0x00, valueToTest};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x00, program, n, 0x00);
      REQUIRE(bus.cpu.getProgramCounter() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedResult);
    }
  }
}

TEST_CASE("BMI - Branch if Negative Set", "[opcode][bmi]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x30;

  SECTION("BMI Program Counter Correct")
  {
    auto [programCounterInitalValue, programOffset, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, uint16_t>({
        {0x00, 0x01, 0x05},
        {0x80, 0x80, 0x04},
        {0x00, 0xFB, 0xFFFF},
        {0x7B, 0x80, 0xFFFF},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:04X}) works",
      bus.cpu.executioner.getOperation(operation), programCounterInitalValue, programOffset, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, 0x80, 0x30, programOffset};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(programCounterInitalValue, program, n, programCounterInitalValue);
      REQUIRE(bus.cpu.getProgramCounter() == programCounterInitalValue);

      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getProgramCounter(), 4) == hex(expectedValue, 4));
    }
  }
}

TEST_CASE("BNE - Branch On Result Not Zero", "[opcode][bne]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0xD0;

  SECTION("BNE Program Counter Correct")
  {
    MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

    auto [programCounterInitalValue, programOffset, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, uint16_t>({
        {0x00, 0x01, 0x05},
        {0x80, 0x80, 0x04},
        {0x00, 0xFB, 0xFFFF},
        {0x7B, 0x80, 0xFFFF},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:04X}) works",
      bus.cpu.executioner.getOperation(operation), programCounterInitalValue, programOffset, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, 0x01, 0xD0, programOffset};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(programCounterInitalValue, program, n, programCounterInitalValue);
      REQUIRE(bus.cpu.getProgramCounter() == programCounterInitalValue);

      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getProgramCounter(), 4) == hex(expectedValue, 4));
    }
  }
}

TEST_CASE("BPL - Branch if Negative Clear", "[opcode][bpl]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x10;

  SECTION("BPL Program Counter Correct")
  {
    auto [programCounterInitalValue, programOffset, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, uint16_t>({
        {0x00, 0x01, 0x05},
        {0x80, 0x80, 0x04},
        {0x00, 0xFB, 0xFFFF},
        {0x7B, 0x80, 0xFFFF},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:04X}) works",
      bus.cpu.executioner.getOperation(operation), programCounterInitalValue, programOffset, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, 0x79, 0x10, programOffset};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(programCounterInitalValue, program, n, programCounterInitalValue);
      REQUIRE(bus.cpu.getProgramCounter() == programCounterInitalValue);

      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getProgramCounter(), 4) == hex(expectedValue, 4));
    }
  }
}

TEST_CASE("BRK - Simulate Interrupt Request (IRQ)", "[opcode][brk]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;
  uint8_t operation = 0x00;

  SECTION("BRK Program Counter Set To Address At Break Vector Address")
  {
    bus.cpu.reset();

    uint8_t program[] = {operation};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);
    REQUIRE(bus.cpu.getProgramCounter() == 0x00);
    bus.cpuWrite(0xFFFE, 0xBC);
    bus.cpuWrite(0xFFFF, 0xCD);

    bus.cpu.clock();
    REQUIRE(bus.cpu.getProgramCounter() == 0xCDBC);
  }

  SECTION("BRK Program Counter Stack Correct")
  {
    bus.cpu.reset();

    uint8_t program[] = {0x00};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0xABCD, program, n, 0xABCD);

    uint8_t stackLocation = bus.cpu.getRegisterSP();

    bus.cpu.clock();

    REQUIRE(hex(bus.cpuRead(stackLocation + (0x0100 - 0)), 2) == hex(0xAB, 2));
    REQUIRE(hex(bus.cpuRead(stackLocation + (0x0100 - 1)), 2) == hex(0xCF, 2));
  }

  SECTION("BRK Stack Pointer Correct")
  {
    bus.cpu.reset();

    uint8_t program[] = {0x00};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0xABCD, program, n, 0xABCD);

    uint8_t stackLocation = bus.cpu.getRegisterSP();
    bus.cpu.clock();

    REQUIRE(hex(bus.cpu.getRegisterSP(), 2) == hex(stackLocation - 3, 2));
  }

  SECTION("BRK Stack Set Flag Operations Correctly")
  {
    auto [sectionOperation, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t>({
        {0x38, 0x31}, //SEC Carry Flag Test
        {0xF8, 0x38}, //SED Decimal Flag Test
        {0x78, 0x34}, //SEI Interrupt Flag Test
      })
    );

    DYNAMIC_SECTION(
      fmt::format("Check if {:s}(0x{:02X}) works",
      bus.cpu.executioner.getOperation(sectionOperation), expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0x58, sectionOperation, 0x00};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x00, program, n, 0x00);
      REQUIRE(bus.cpu.getProgramCounter() == 0x00);

      uint8_t stackLocation = bus.cpu.getRegisterSP();

      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(hex(bus.cpuRead(stackLocation + (0x0100 - 2)), 2) == hex(expectedValue, 2));
    }
  }

  SECTION("BRK Stack Non Set Flag Operations Correctly")
  {
    auto [accumulatorValue, memoryValue, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t>({
        {0x01, 0x80, 0xB0}, //Negative
        {0x01, 0x7F, 0xF0}, //Overflow + Negative
        {0x00, 0x00, 0x32}, //Zero
      })
    );

    DYNAMIC_SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0x58, 0xA9, accumulatorValue, 0x69, memoryValue, 0x00};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x00, program, n, 0x00);
      REQUIRE(bus.cpu.getProgramCounter() == 0x00);

      uint8_t stackLocation = bus.cpu.getRegisterSP();

      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(hex(bus.cpuRead(stackLocation + (0x0100 - 2)), 2) == hex(expectedValue, 2));
    }
  }
}

TEST_CASE("BVC - Branch if Overflow Clear", "[opcode][bvc]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x50;

  SECTION("BVC Program Counter Correct")
  {
    MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

    auto [programCounterInitalValue, programOffset, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, uint16_t>({
        {0x00, 0x01, 0x03},
        {0x80, 0x80, 0x02},
        {0x00, 0xFD, 0xFFFF},
        {0x7D, 0x80, 0xFFFF},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:04X}) works",
      bus.cpu.executioner.getOperation(operation), programCounterInitalValue, programOffset, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0x50, programOffset};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(programCounterInitalValue, program, n, programCounterInitalValue);
      REQUIRE(bus.cpu.getProgramCounter() == programCounterInitalValue);

      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getProgramCounter(), 4) == hex(expectedValue, 4));
    }
  }
}

TEST_CASE("BVS - Branch if Overflow Set", "[opcode][bvs]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x70;

  SECTION("BVS Program Counter Correct")
  {
    auto [programCounterInitalValue, programOffset, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, uint16_t>({
        {0x00, 0x01, 0x07},
        {0x80, 0x80, 0x06},
        {0x00, 0xF9, 0xFFFF},
        {0x79, 0x80, 0xFFFF},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:04X}) works",
      bus.cpu.executioner.getOperation(operation), programCounterInitalValue, programOffset, expectedValue)
    ) {
      bus.cpu.reset();

      //std::string program;
      //fmt::format_to(std::back_inserter(program), "A9 01 69 7F 70 {:02X}", programOffset);
      //bus.cpu.loadProgram(programCounterInitalValue, program, programCounterInitalValue);

      uint8_t program[] = {0xA9, 0x01, 0x69, 0x7F, 0x70, programOffset};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(programCounterInitalValue, program, n, programCounterInitalValue);

      REQUIRE(bus.cpu.getProgramCounter() == programCounterInitalValue);

      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getProgramCounter(), 4) == hex(expectedValue, 4));
    }
  }
}

TEST_CASE("CLC - Clear Carry Flag", "[opcode][clc]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x18;

  SECTION("CLC Carry Flag Cleared Correctly")
  {
    bus.cpu.reset();

    uint8_t program[] = {0x18};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);

    bus.cpu.clock();
    REQUIRE(bus.cpu.getFlag(bus.cpu.C) == 0);
  }
}

TEST_CASE("CLD - Clear Decimal Flag", "[opcode][cld]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0xD8;

  SECTION("CLD Carry Flag Set And Cleared Correctly")
  {
    bus.cpu.reset();

    uint8_t program[] = {0xF8, 0xD8};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);

    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(bus.cpu.getFlag(bus.cpu.D) == 0);
  }
}

TEST_CASE("CLI - Clear Interrupt Flag", "[opcode][cli]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x58;

  SECTION("CLI Interrupt Flag Cleared Correctly")
  {
    bus.cpu.reset();

    uint8_t program[] = {0x58};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);

    bus.cpu.clock();
    REQUIRE(bus.cpu.getFlag(bus.cpu.I) == 0);
  }
}

TEST_CASE("CLV - Clear Overflow Flag", "[opcode][clv]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x88;

  SECTION("CLV Overflow Flag Cleared Correctly")
  {
    bus.cpu.reset();

    uint8_t program[] = {0xB8};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);

    bus.cpu.clock();
    REQUIRE(bus.cpu.getFlag(bus.cpu.V) == 0);
  }
}

TEST_CASE("CMP - Compare Memory With Accumulator", "[opcode][cmp]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0xC9;

  SECTION("CMP Zero Flag Set When Values Match")
  {
    auto [accumulatorValue, memoryValue, expectedResult] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t>({
        {0x00, 0x00, 1},
        {0xFF, 0x00, 0},
        {0x00, 0xFF, 0},
        {0xFF, 0xFF, 1},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorValue, 0xC9, memoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x00, program, n, 0x00);
      REQUIRE(bus.cpu.getProgramCounter() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedResult);
    }
  }

  SECTION("CMP Carry Flag Set When Accumulator Is Greater Than Or Equal")
  {
    auto [accumulatorValue, memoryValue, expectedResult] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t>({
        {0x00, 0x00, 1},
        {0xFF, 0x00, 1},
        {0x00, 0xFF, 0},
        {0x00, 0x01, 0},
        {0xFF, 0xFF, 1},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorValue, 0xC9, memoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x00, program, n, 0x00);
      REQUIRE(bus.cpu.getProgramCounter() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.C) == expectedResult);
    }
  }

  SECTION("CMP Negative Flag Set When Result Is Negative")
  {
    auto [accumulatorValue, memoryValue, expectedResult] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t>({
        {0xFE, 0xFF, 1},
        {0x81, 0x1, 1},
        {0x81, 0x2, 0},
        {0x79, 0x1, 0},
        {0x00, 0x1, 1},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorValue, 0xC9, memoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x00, program, n, 0x00);
      REQUIRE(bus.cpu.getProgramCounter() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedResult);
    }
  }
}

TEST_CASE("CPX - Compare Memory With X Register", "[opcode][cpx]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0xE0;

  SECTION("CPX Zero Flag Set When Values Match")
  {
    auto [accumulatorValue, memoryValue, expectedResult] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t>({
        {0x00, 0x00, 1},
        {0xFF, 0x00, 0},
        {0x00, 0xFF, 0},
        {0xFF, 0xFF, 1},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA2, accumulatorValue, 0xE0, memoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x00, program, n, 0x00);
      REQUIRE(bus.cpu.getProgramCounter() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedResult);
    }
  }

  SECTION("CPX Carry Flag Set When Accumulator Is Greater Than Or Equal")
  {
    auto [accumulatorValue, memoryValue, expectedResult] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t>({
        {0x00, 0x00, 1},
        {0xFF, 0x00, 1},
        {0x00, 0xFF, 0},
        {0x00, 0x01, 0},
        {0xFF, 0xFF, 1},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA2, accumulatorValue, 0xE0, memoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x00, program, n, 0x00);
      REQUIRE(bus.cpu.getProgramCounter() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.C) == expectedResult);
    }
  }

  SECTION("CPX Negative Flag Set When Result Is Negative")
  {
    auto [accumulatorValue, memoryValue, expectedResult] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t>({
        {0xFE, 0xFF, 1},
        {0x81, 0x1, 1},
        {0x81, 0x2, 0},
        {0x79, 0x1, 0},
        {0x00, 0x1, 1},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA2, accumulatorValue, 0xE0, memoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x00, program, n, 0x00);
      REQUIRE(bus.cpu.getProgramCounter() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedResult);
    }
  }
}

TEST_CASE("CPY - Compare Memory With X Register", "[opcode][cpy]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0xC0;

  SECTION("CPY Zero Flag Set When Values Match")
  {
    auto [accumulatorValue, memoryValue, expectedResult] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t>({
        {0x00, 0x00, 1},
        {0xFF, 0x00, 0},
        {0x00, 0xFF, 0},
        {0xFF, 0xFF, 1},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA0, accumulatorValue, 0xC0, memoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x00, program, n, 0x00);
      REQUIRE(bus.cpu.getProgramCounter() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedResult);
    }
  }

  SECTION("CPY Carry Flag Set When Accumulator Is Greater Than Or Equal")
  {
    auto [accumulatorValue, memoryValue, expectedResult] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t>({
        {0x00, 0x00, 1},
        {0xFF, 0x00, 1},
        {0x00, 0xFF, 0},
        {0x00, 0x01, 0},
        {0xFF, 0xFF, 1},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA0, accumulatorValue, 0xC0, memoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x00, program, n, 0x00);
      REQUIRE(bus.cpu.getProgramCounter() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.C) == expectedResult);
    }
  }

  SECTION("CPY Negative Flag Set When Result Is Negative")
  {
    auto [accumulatorValue, memoryValue, expectedResult] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t>({
        {0xFE, 0xFF, 1},
        {0x81, 0x1, 1},
        {0x81, 0x2, 0},
        {0x79, 0x1, 0},
        {0x00, 0x1, 1},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA0, accumulatorValue, 0xC0, memoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x00, program, n, 0x00);
      REQUIRE(bus.cpu.getProgramCounter() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedResult);
    }
  }
}

TEST_CASE("DEC - Decrement Memory by One", "[opcode][dec]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0xC6;

  SECTION("DEC Memory Has Correct Value")
  {
    auto [initialMemoryValue, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x00, 0xFF},
      {0xFF, 0xFE},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), initialMemoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xC6, 0x03, 0x00, initialMemoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      
      bus.cpu.clock();
      REQUIRE(bus.cpuRead(0x0003, false) == expectedValue);
    }
  }

  SECTION("DEC Zero Set Correctly")
  {
    auto [initialMemoryValue, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x00, 0},
      {0x01, 1},
      {0x02, 0},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), initialMemoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xC6, 0x03, 0x00, initialMemoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedValue);
    }
  }

  SECTION("DEC Negative Set Correctly")
  {
    auto [initialMemoryValue, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x80, 0},
      {0x81, 1},
      {0x00, 1}
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), initialMemoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xC6, 0x03, 0x00, initialMemoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedValue);
    }
  }
}

TEST_CASE("DEX - Decrement X by One", "[opcode][dex]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0xCA;

  SECTION("DEX Memory Has Correct Value")
  {
    auto [initialMemoryValue, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x00, 0xFF},
      {0xFF, 0xFE},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), initialMemoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterX() == 0x00);

      uint8_t program[] = {0xA2, initialMemoryValue, 0xCA};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      
      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(bus.cpu.getRegisterX() == expectedValue);
    }
  }

  SECTION("DEX Zero Set Correctly")
  {
    auto [initialMemoryValue, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x00, 0},
      {0x01, 1},
      {0x02, 0},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), initialMemoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA2, initialMemoryValue, 0xCA};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      
      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedValue);
    }
  }

  SECTION("DEX Negative Set Correctly", "[opcode][dex]")
  {
    auto [initialMemoryValue, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x80, 0},
      {0x81, 1},
      {0x00, 1}
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), initialMemoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA2, initialMemoryValue, 0xCA};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedValue);
    }
  }
}

TEST_CASE("DEY - Decrement Y by One", "[opcode][dey]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x88;

  SECTION("DEY Memory Has Correct Value")
  {
    auto [initialMemoryValue, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x00, 0xFF},
      {0xFF, 0xFE},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), initialMemoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterY() == 0x00);

      uint8_t program[] = {0xA0, initialMemoryValue, 0x88};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      
      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getRegisterY(), 2) == hex(expectedValue, 2));
    }
  }

  SECTION("DEY Zero Set Correctly")
  {
    auto [initialMemoryValue, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x00, 0},
      {0x01, 1},
      {0x02, 0},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), initialMemoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA0, initialMemoryValue, 0x88};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      
      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedValue);
    }
  }

  SECTION("DEY Negative Set Correctly")
  {
    auto [initialMemoryValue, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x80, 0},
      {0x81, 1},
      {0x00, 1}
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), initialMemoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA0, initialMemoryValue, 0x88};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedValue);
    }
  }
}

TEST_CASE("EOR - Exclusive OR Compare Accumulator With Memory", "[opcode][eor]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x49;

  SECTION("EOR Accumulator Correct")
  {
    auto [accumulatorValue, memoryValue, expectedResult] = GENERATE( table<uint8_t, uint8_t, uint8_t>({
      {0x00, 0x00, 0x00},
      {0xFF, 0x00, 0xFF},
      {0x00, 0xFF, 0xFF},
      {0x55, 0xAA, 0xFF},
      {0xFF, 0xFF, 0x00},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorValue, 0x49, memoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      
      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(bus.cpu.getRegisterAC() == expectedResult);
    }
  }

  SECTION("EOR Negative Flag Correct")
  {
    auto [accumulatorValue, memoryValue, expectedResult] = GENERATE( table<uint8_t, uint8_t, uint8_t>({
      {0xFF, 0xFF, 0},
      {0x80, 0x7F, 1},
      {0x40, 0x3F, 0},
      {0xFF, 0x7F, 1},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorValue, 0x49, memoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      
      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedResult);
    }
  }

  SECTION("EOR Zero Flag Correct")
  {
    auto [accumulatorValue, memoryValue, expectedResult] = GENERATE( table<uint8_t, uint8_t, uint8_t>({
      {0xFF, 0xFF, 1},
      {0x80, 0x7F, 0},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorValue, 0x49, memoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      
      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedResult);
    }
  }
}

TEST_CASE("INC - Increment Memory by One", "[opcode][inc]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0xE6;

  SECTION("INC Memory Has Correct Value")
  {
    auto [initialMemoryValue, expectedValue] = GENERATE( table<uint8_t, bool>({
      {0x00, 0x01},
      {0xFF, 0x00},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), initialMemoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xE6, 0x03, 0x00, initialMemoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      
      bus.cpu.clock();
      REQUIRE(bus.cpuRead(0x0003, false) == expectedValue);
    }
  }

  SECTION("INC Zero Set Correctly")
  {
    auto [initialMemoryValue, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x00, 0},
      {0xFF, 1},
      {0xFE, 0},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), initialMemoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xE6, 0x03, 0x00, initialMemoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedValue);
    }
  }

  SECTION("INC Negative Set Correctly")
  {
    auto [initialMemoryValue, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x78, 0},
      {0x80, 1},
      {0x00, 0}
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), initialMemoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xE6, 0x03, 0x00, initialMemoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedValue);
    }
  }
}

TEST_CASE("INX - Increment X by One", "[opcode][inx]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0xE8;

  SECTION("INX Memory Has Correct Value")
  {
    MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

    auto [initialMemoryValue, expectedValue] = GENERATE( table<uint8_t, bool>({
      {0x00, 0x01},
      {0xFF, 0x00},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), initialMemoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA2, initialMemoryValue, 0xE8};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      
      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(bus.cpu.getRegisterX() == expectedValue);
    }
  }

  SECTION("INX Zero Set Correctly")
  {
    auto [initialMemoryValue, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x00, 0},
      {0xFF, 1},
      {0xFE, 0},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), initialMemoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA2, initialMemoryValue, 0xE8};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      
      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedValue);
    }
  }

  SECTION("INX Negative Set Correctly")
  {
    auto [initialMemoryValue, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x78, 0},
      {0x80, 1},
      {0x00, 0}
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), initialMemoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA2, initialMemoryValue, 0xE8};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedValue);
    }
  }
}

TEST_CASE("INY - Increment Y by One", "[opcode][iny]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0xC8;

  SECTION("INY Memory Has Correct Value")
  {
    auto [initialMemoryValue, expectedValue] = GENERATE( table<uint8_t, bool>({
      {0x00, 0x01},
      {0xFF, 0x00},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), initialMemoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA0, initialMemoryValue, 0xC8};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      
      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(bus.cpu.getRegisterY() == expectedValue);
    }
  }

  SECTION("INY Zero Set Correctly")
  {
    auto [initialMemoryValue, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x00, 0},
      {0xFF, 1},
      {0xFE, 0},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), initialMemoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA0, initialMemoryValue, 0xC8};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      
      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedValue);
    }
  }

  SECTION("INY Negative Set Correctly")
  {
    auto [initialMemoryValue, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x78, 0},
      {0x80, 1},
      {0x00, 0}
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), initialMemoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA0, initialMemoryValue, 0xC8};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedValue);
    }
  }
}

TEST_CASE("JMP - Jump to New Location", "[opcode][jmp]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  SECTION("Program Counter Set Correctly After Jump")
  {
    bus.cpu.reset();

    uint8_t program[] = {0x4C, 0x08, 0x00};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);

    bus.cpu.clock();
    REQUIRE(hex(bus.cpu.getProgramCounter(), 4) == hex(0x08, 4));
  }

  SECTION("Program Counter Set Correctly After Indirect Jump")
  {
    bus.cpu.reset();

    uint8_t program[] = {0x6C, 0x03, 0x00, 0x08, 0x00};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);

    bus.cpu.clock();
    REQUIRE(hex(bus.cpu.getProgramCounter(), 4) == hex(0x08, 4));
  }

#ifndef EMULATE65C02
  SECTION("Indirect Wraps Correct If MSB IS FF (6502 only)")
  {
    bus.cpu.reset();

    uint8_t program[] = {0x6C, 0xFF, 0x01, 0x08, 0x00};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);
    bus.cpuWrite(0x01FE, 0x6C);
    bus.cpuWrite(0x01FF, 0x03);
    bus.cpuWrite(0x0100, 0x02);

    bus.cpu.clock();
    REQUIRE(hex(bus.cpu.getProgramCounter(), 4) == hex(0x0203, 4));
  }
#endif
}

TEST_CASE("JSR - Jump to SubRoutine", "[opcode][jsr]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  SECTION("Stack Loads Correct Value")
  {
    bus.cpu.reset();

    uint8_t program[] = {0x20, 0xCC, 0xCC};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0xBBAA, program, n, 0xBBAA);

    uint8_t stackLocation = bus.cpu.getRegisterSP();
    bus.cpu.clock();

    REQUIRE(hex(bus.cpuRead(stackLocation + 0x0100), 4) == hex(0xBB, 4));
    REQUIRE(hex(bus.cpuRead(stackLocation + 0x0100 - 1), 4) == hex(0xAC, 4));
  }

  SECTION("Program Counter Correct")
  {
    bus.cpu.reset();

    uint8_t program[] = {0x20, 0xCC, 0xCC};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);
    bus.cpu.clock();

    REQUIRE(hex(bus.cpu.getProgramCounter(), 4) == hex(0xCCCC, 4));
  }

  SECTION("Stack Pointer Correct")
  {
    bus.cpu.reset();

    uint8_t program[] = {0x20, 0xCC, 0xCC};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0xBBAA, program, n, 0xBBAA);

    uint8_t stackLocation = bus.cpu.getRegisterSP();
    bus.cpu.clock();

    REQUIRE(hex(bus.cpu.getRegisterSP(), 4) == hex(stackLocation - 2, 4));
  }
}

TEST_CASE("LDA - Load Accumulator with Memory", "[opcode][lda]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0xA9;

  SECTION("LDA Accumulator Has Correct Value")
  {
    bus.cpu.reset();

    REQUIRE(bus.cpu.getRegisterAC() == 0x00);

    uint8_t program[] = {0xA9, 0x03};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);

    bus.cpu.clock();
    REQUIRE(bus.cpu.getRegisterAC() == 0x03);
  }

  SECTION("LDA Zero Set Correctly")
  {
    MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

    auto [valueToLoad, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x00, 1},
      {0x03, 0},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), valueToLoad, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      uint8_t program[] = {0xA9, valueToLoad};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedValue);
    }
  }

  SECTION("LDA Negative Set Correctly")
  {
    auto [valueToLoad, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x00, 0},
      {0x79, 0},
      {0x80, 1},
      {0xFF, 1}
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), valueToLoad, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      uint8_t program[] = {0xA9, valueToLoad};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedValue);
    }
  }
}

TEST_CASE("LDX - Load X with Memory", "[opcode][ldx]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0xA2;

  SECTION("LDX X-Register Has Correct Value")
  {
    MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

    MainBus bus;

    bus.cpu.reset();

    REQUIRE(bus.cpu.getRegisterX() == 0x00);

    uint8_t program[] = {0xA2, 0x03};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);

    bus.cpu.clock();
    REQUIRE(bus.cpu.getRegisterX() == 0x03);
  }

  SECTION("LDX Zero Set Correctly")
  {
    auto [valueToLoad, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x00, 1},
      {0x03, 0},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), valueToLoad, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterX() == 0x00);

      uint8_t program[] = {0xA2, valueToLoad};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedValue);
    }
  }

  SECTION("LDX Negative Set Correctly")
  {
    auto [valueToLoad, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x00, 0},
      {0x79, 0},
      {0x80, 1},
      {0xFF, 1}
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), valueToLoad, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterX() == 0x00);

      uint8_t program[] = {0xA2, valueToLoad};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedValue);
    }
  }
}

TEST_CASE("LDY - Load Y with Memory", "[opcode][ldy]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0xA0;

  SECTION("LDY Y-Register Has Correct Value")
  {
    bus.cpu.reset();

    REQUIRE(bus.cpu.getRegisterY() == 0x00);

    uint8_t program[] = {0xA0, 0x03};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);

    bus.cpu.clock();
    REQUIRE(bus.cpu.getRegisterY() == 0x03);
  }

  SECTION("LDY Zero Set Correctly")
  {
    auto [valueToLoad, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x00, 1},
      {0x03, 0},
    }));

    DYNAMIC_SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), valueToLoad, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterY() == 0x00);

      uint8_t program[] = {0xA0, valueToLoad};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedValue);
    }
  }

  SECTION("LDY Negative Set Correctly")
  {
    MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

    auto [valueToLoad, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x00, 0},
      {0x79, 0},
      {0x80, 1},
      {0xFF, 1}
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), valueToLoad, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterY() == 0x00);

      uint8_t program[] = {0xA0, valueToLoad};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedValue);
    }
  }
}

TEST_CASE("LSR - Logical Shift Right", "[opcode][lsr]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x4A;

  SECTION("LSR Negative Set Correctly")
  {
    auto [accumulatorValue, carryBitSet] = GENERATE( table<uint8_t, bool>({
      {0xFF, false},
      {0xFE, false},
      {0xFF, true},
      {0x00, true},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, carryBitSet)
    ) {
      bus.cpu.reset();

      uint8_t carryOperation = carryBitSet ? 0x38 : 0x18;

      uint8_t program[] = {carryOperation, 0xA9, accumulatorValue, 0x4A};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == 0);
    }
  }

  SECTION("LSR Zero Set Correctly")
  {
    auto [accumulatorValue, expectedValue] = GENERATE( table<uint8_t, uint8_t>({
      {0x01, 1},
      {0x02, 0}
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorValue, 0x4A};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedValue);
    }
  }

  SECTION("LSR Carry Set Correctly")
  {
    auto [accumulatorValue, expectedValue] = GENERATE( table<uint8_t, bool>({
      {0x01, true},
      {0x02, false}
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorValue, 0x4A};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.C) == expectedValue);
    }
  }

  SECTION("LSR Correct Value Stored")
  {
    auto [operation, valueToShift, expectedValue, expectedLocation] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
        {0x4A, 0xFF, 0x7F, 0x00}, // LSR Accumulator
        {0x4A, 0xFD, 0x7E, 0x00}, // LSR Accumulator
        {0x46, 0xFF, 0x7F, 0x01}, // LSR Zero Page
        {0x56, 0xFF, 0x7F, 0x01}, // LSR Zero Page X
        {0x4E, 0xFF, 0x7F, 0x01}, // LSR Absolute
        {0x5E, 0xFF, 0x7F, 0x01}, // LSR Absolute X
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), valueToShift, expectedValue, expectedLocation)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, valueToShift, operation, expectedLocation};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      uint8_t actualValue;
      if (operation == 0x4A)
      {
        actualValue = bus.cpu.getRegisterAC();
      }
      else
      {
        actualValue = bus.cpuRead(expectedLocation);
      }
      REQUIRE(actualValue == expectedValue);
    }
  }
}

TEST_CASE("ORA - Bitwise OR Compare Memory with Accumulator", "[opcode][ora]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x09;

  SECTION("ORA Accumulator Correct")
  {
    auto [accumulatorValue, memoryValue, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t>({
        {0x00, 0x00, 0x00},
        {0xFF, 0xFF, 0xFF},
        {0x55, 0xAA, 0xFF},
        {0xAA, 0x55, 0xFF},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorValue, 0x09, memoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(expectedValue, 2));
    }
  }

  SECTION("ORA Zero Set Correctly")
  {
    auto [accumulatorValue, memoryValue, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t>({
      {0x00, 0x00, 1},
      {0xFF, 0xFF, 0},
      {0x00, 0x01, 0}
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorValue, 0x09, memoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedValue);
    }
  }

  SECTION("ORA Negative Set Correctly")
  {
    auto [accumulatorValue, memoryValue, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t>({
        {0x7F, 0x80, 1},
        {0x79, 0x00, 0},
        {0xFF, 0xFF, 1},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorValue, 0x09, memoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedValue);
    }
  }
}

TEST_CASE("PHA - Push Accumulator Onto Stack", "[opcode][pha]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x48;

  SECTION("Stack Has Correct Value")
  {
    uint8_t program[] = {0xA9, 0x03, 0x48};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);

    uint8_t stackLocation = bus.cpu.getRegisterSP();

    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(hex(bus.cpuRead(stackLocation + 0x0100), 2) == hex(0x03, 2));
  }

  SECTION("Stack Pointer Has Correct Value")
  {
    uint8_t program[] = {0xA9, 0x03, 0x48};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);

    uint8_t stackLocation = bus.cpu.getRegisterSP();

    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(hex(bus.cpu.getRegisterSP(), 2) == hex(stackLocation - 1, 2));
  }

  SECTION("Stack Pointer Has Correct Value When Wrapping")
  {
    uint8_t program[] = {0x9A, 0x48};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);

    uint8_t stackLocation = bus.cpu.getRegisterSP();

    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(hex(bus.cpu.getRegisterSP(), 2) == hex(0xFF, 2));
  }
}

TEST_CASE("PHP - Push Flags Onto Stack", "[opcode][php]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x08;

  SECTION("PHP Stack Set Flag Operations Correctly")
  {
    auto [operation, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t>({
        {0x38, 0x31},
        {0xF8, 0x38},
        {0x78, 0x34},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0x58, operation, 0x08};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      uint8_t stackLocation = bus.cpu.getRegisterSP();

      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(hex(bus.cpuRead(stackLocation + 0x0100), 2) == hex(expectedValue, 2));
    }
  }

  SECTION("PHP Stack Non Set Flag Operations Correctly", "[opcode][php]")
  {
    auto [accumulatorValue, memoryValue, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t>({
        {0x01, 0x80, 0xB0}, //Negative
        {0x01, 0x7F, 0xF0}, //Overflow + Negative
        {0x00, 0x00, 0x32}, //Zero
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0x58, 0xA9, accumulatorValue, 0x69, memoryValue, 0x08};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      uint8_t stackLocation = bus.cpu.getRegisterSP();
      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(hex(bus.cpuRead(stackLocation + 0x0100), 2) == hex(expectedValue, 2));
    }
  }

  SECTION("PHP Stack Pointer Has Correct Value")
  {
    bus.cpu.reset();

    uint8_t program[] = {0x08};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);

    uint8_t stackLocation = bus.cpu.getRegisterSP();

    bus.cpu.clock();

    REQUIRE(hex(bus.cpu.getRegisterSP(), 2) == hex(stackLocation - 1, 2));
  }
}

TEST_CASE("PLA - Pull From Stack to Accumulator", "[opcode][pla]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x68;

  SECTION("PLA Accumulator Has Correct Value")
  {
    bus.cpu.reset();

    uint8_t program[] = {0xA9, 0x03, 0x48, 0xA9, 0x00, 0x68};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);

    bus.cpu.clock();

    REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(0x03, 2));
  }

  SECTION("PLA Zero Flag Has Correct Value", "[opcode][pla]")
  {
    auto [valueToLoad, expectedResult] = 
      GENERATE( table<uint8_t, uint8_t>({
        {0x00, 1},
        {0x01, 0},
        {0xFF, 0},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), valueToLoad, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, valueToLoad, 0x48, 0x68};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      //uint8_t stackLocation = bus.cpu.getRegisterSP();
      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedResult);
    }
  }

  SECTION("PLA Negative Flag Has Correct Value", "[opcode][pla]")
  {
    auto [valueToLoad, expectedResult] = 
      GENERATE( table<uint8_t, uint8_t>({
        {0x7F, 0},
        {0x80, 1},
        {0xFF, 1},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), valueToLoad, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, valueToLoad, 0x48, 0x68};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      //uint8_t stackLocation = bus.cpu.getRegisterSP();
      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedResult);
    }
  }
}

TEST_CASE("PLP - Pull From Stack to Flags", "[opcode][plp]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x28;

  SECTION("Carry Flag Set Correctly")
  {
    MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

    bus.cpu.reset();

    uint8_t program[] = {0xA9, 0x01, 0x48, 0x28};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);

    bus.cpu.clock();
    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(bus.cpu.getFlag(bus.cpu.C) == 1);
  }

  SECTION("Zero Flag Set Correctly")
  {
    MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

    bus.cpu.reset();

    uint8_t program[] = {0xA9, 0x02, 0x48, 0x28};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);

    bus.cpu.clock();
    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == 1);
  }

  SECTION("Decimal Flag Set Correctly")
  {
    MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

    bus.cpu.reset();

    uint8_t program[] = {0xA9, 0x08, 0x48, 0x28};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);

    bus.cpu.clock();
    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(bus.cpu.getFlag(bus.cpu.D) == 1);
  }

  SECTION("Interrupt Flag Set Correctly")
  {
    MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

    bus.cpu.reset();

    uint8_t program[] = {0xA9, 0x04, 0x48, 0x28};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);

    bus.cpu.clock();
    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(bus.cpu.getFlag(bus.cpu.I) == 1);
  }

  SECTION("Overflow Flag Set Correctly")
  {
    MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

    bus.cpu.reset();

    uint8_t program[] = {0xA9, 0x40, 0x48, 0x28};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);

    bus.cpu.clock();
    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(bus.cpu.getFlag(bus.cpu.V) == 1);
  }

  SECTION("Negative Flag Set Correctly")
  {
    MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

    bus.cpu.reset();

    uint8_t program[] = {0xA9, 0x80, 0x48, 0x28};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);

    bus.cpu.clock();
    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(bus.cpu.getFlag(bus.cpu.N) == 1);
  }
}

TEST_CASE("ROL - Rotate Left", "[opcode][rol]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x2A;

  SECTION("ROL Negative Set Correctly")
  {
    auto [accumulatorValue, expectedResult] = 
      GENERATE( table<uint8_t, uint8_t>({
        {0x40, 1},
        {0x3F, 0},
        {0x80, 0},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorValue, 0x2A};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedResult);
    }
  }

  SECTION("ROL Zero Set Correctly")
  {
    auto [carryFlagSet, expectedResult] = 
      GENERATE( table<bool, uint8_t>({
        {true, 0},
        {false, 1},
      })
    );

    SECTION(
      fmt::format("Check if {:s}({}, {}) works",
      bus.cpu.executioner.getOperation(operation), carryFlagSet, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t carryOperation = carryFlagSet ? 0x38 : 0x18;

      uint8_t program[] = {carryOperation, 0x2A};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedResult);
    }
  }

  SECTION("ROL Carry Flag Set Correctly")
  {
    auto [accumulatorValue, expectedResult] = 
      GENERATE( table<uint8_t, uint8_t>({
        {0x80, 1},
        {0x7F, 0},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorValue, 0x2A};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.C) == expectedResult);
    }
  }

  SECTION("ROL Correct Value Stored")
  {
    auto [operation, valueToRotate, expectedValue, expectedLocation] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
        {0x2A, 0x55, 0xAA, 0x00}, // ROL Accumulator
        {0x2A, 0x55, 0xAA, 0x00}, // ROL Accumulator
        {0x26, 0x55, 0xAA, 0x01}, // ROL Zero Page
        {0x36, 0x55, 0xAA, 0x01}, // ROL Zero Page X
        {0x2E, 0x55, 0xAA, 0x01}, // ROL Absolute
        {0x3E, 0x55, 0xAA, 0x01}, // ROL Absolute X
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), valueToRotate, expectedValue, expectedLocation)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getProgramCounter() == 0x00);

      uint8_t program[] = {0xA9, valueToRotate, operation, expectedLocation};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      uint8_t actualResult;
      if (operation == 0x2A)
      {
        actualResult = bus.cpu.getRegisterAC();
      }
      else
      {
        actualResult = bus.cpuRead(expectedLocation);
      }

      REQUIRE(hex(actualResult, 2) == hex(expectedValue, 2));
    }
  }
}

TEST_CASE("ROR - Rotate Right", "[opcode][ror]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x6A;

  SECTION("ROR Negative Set Correctly")
  {
    auto [accumulatorValue, carryBitSet, expectedValue] = 
      GENERATE( table<uint8_t, bool, bool>({
        {0xFF, false, false},
        {0xFE, false, false},
        {0xFF, true, true},
        {0x00, true, true},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, carryBitSet, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t carryOperation = carryBitSet ? 0x38 : 0x18;

      uint8_t program[] = {carryOperation, 0xA9, accumulatorValue, 0x6A};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedValue);
    }
  }

  SECTION("ROR Zero Set Correctly")
  {
    auto [accumulatorValue, carryBitSet, expectedResult] = 
      GENERATE( table<uint8_t, bool, uint8_t>({
        {0x00, false, 1},
        {0x00, true, 0},
        {0x01, false, 1},
        {0x01, true, 0},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, carryBitSet, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t carryOperation = carryBitSet ? 0x38 : 0x18;

      uint8_t program[] = {carryOperation, 0xA9, accumulatorValue, 0x6A};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedResult);
    }
  }

  SECTION("ROR Carry Flag Set Correctly", "[opcode][ror]")
  {
    auto [accumulatorValue, expectedResult] = 
      GENERATE( table<uint8_t, uint8_t>({
        {0x01, 1},
        {0x02, 0},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorValue, 0x6A};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.C) == expectedResult);
    }
  }

  SECTION("ROR Correct Value Stored", "[opcode][ror]")
  {
    auto [operation, valueToRotate, expectedValue, expectedLocation] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
        {0x6A, 0xAA, 0x55, 0x00}, // ROR Accumulator
        {0x6A, 0xAA, 0x55, 0x00}, // ROR Accumulator
        {0x66, 0xAA, 0x55, 0x01}, // ROR Zero Page
        {0x76, 0xAA, 0x55, 0x01}, // ROR Zero Page X
        {0x6E, 0xAA, 0x55, 0x01}, // ROR Absolute
        {0x7E, 0xAA, 0x55, 0x01}, // ROR Absolute X
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), valueToRotate, expectedValue, expectedLocation)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getProgramCounter() == 0x00);

      uint8_t program[] = {0xA9, valueToRotate, operation, expectedLocation};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      uint8_t actualResult;
      if (operation == 0x6A)
      {
        actualResult = bus.cpu.getRegisterAC();
      }
      else
      {
        actualResult = bus.cpuRead(expectedLocation);
      }

      REQUIRE(hex(actualResult, 2) == hex(expectedValue, 2));
    }
  }
}

TEST_CASE("RTI - Return from Interrupt", "[opcode][rti]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  SECTION("Program Counter Correct")
  {
    bus.cpu.reset();

    uint8_t program[] = {0x00};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0xABCD, program, n, 0xABCD);

    //The Reset Vector Points to 0x0000 by default, so load the RTI instruction there.
    bus.cpuWrite(0x00, 0x40);

    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(hex(bus.cpu.getProgramCounter(), 4) == hex(0xABCF, 4));
  }

  SECTION("Carry Flag Set Correctly")
  {
    bus.cpu.reset();

    //Load Accumulator and Transfer to Stack, Clear Accumulator, and Return from Interrupt
    uint8_t program[] = {0xA9, 0x01, 0x48, 0x40};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);

    bus.cpu.clock();
    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(bus.cpu.getFlag(bus.cpu.C) == 1);
  }

  SECTION("Zero Flag Set Correctly")
  {
    bus.cpu.reset();

    //Load Accumulator and Transfer to Stack, Clear Accumulator, and Return from Interrupt
    uint8_t program[] = {0xA9, 0x02, 0x48, 0x40};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);

    bus.cpu.clock();
    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == 1);
  }

  SECTION("Interrupt Flag Set Correctly")
  {
    bus.cpu.reset();

    //Load Accumulator and Transfer to Stack, Clear Accumulator, and Return from Interrupt
    uint8_t program[] = {0xA9, 0x04, 0x48, 0x40};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);

    bus.cpu.clock();
    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(bus.cpu.getFlag(bus.cpu.I) == 1);
  }

  SECTION("Decimal Flag Set Correctly")
  {
    MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

    bus.cpu.reset();

    //Load Accumulator and Transfer to Stack, Clear Accumulator, and Return from Interrupt
    uint8_t program[] = {0xA9, 0x08, 0x48, 0x40};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);

    bus.cpu.clock();
    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(bus.cpu.getFlag(bus.cpu.D) == 1);
  }

  SECTION("Overflow Flag Set Correctly")
  {
    bus.cpu.reset();

    //Load Accumulator and Transfer to Stack, Clear Accumulator, and Return from Interrupt
    //uint8_t program[] = {0xA9, 0x40, 0x48, 0x28};
    uint8_t program[] = {0xA9, 0x40, 0x48, 0x40};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);

    bus.cpu.clock();
    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(bus.cpu.getFlag(bus.cpu.V) == 1);
  }

  SECTION("Negative Flag Set Correctly")
  {
    MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

    bus.cpu.reset();

    uint8_t program[] = {0xA9, 0x80, 0x48, 0x28};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);

    bus.cpu.clock();
    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(bus.cpu.getFlag(bus.cpu.N) == 1);
    }
}

TEST_CASE("RTS - Return from SubRoutine", "[opcode][rts]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  SECTION("Program Counter Has Correct Value")
  {
    bus.cpu.reset();

    uint8_t program[] = {0x20, 0x04, 0x00, 0x00, 0x60};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);

    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(hex(bus.cpu.getProgramCounter(), 4) == hex(0x03, 4));
  }

  SECTION("Stack Pointer Has Correct Value")
  {
    MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

    bus.cpu.reset();

    uint8_t program[] = {0x60};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0xBBAA, program, n, 0xBBAA);

    uint8_t stackLocation = bus.cpu.getRegisterSP();

    bus.cpu.clock();

    REQUIRE(hex(bus.cpu.getRegisterSP(), 4) == hex((stackLocation + 2) & 0x00FF, 4));
  }
}

TEST_CASE("SBC - Subtraction With Borrow", "[opcode][sbc]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0xE9;

  SECTION("SBC Accumulator Correct When Not In BDC Mode")
  {
    MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

    auto [accumulatorInitialValue, amountToSubtract, CarryFlagSet, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, bool, uint8_t>({
        {0x00, 0x00, false, 0xFF},
        {0x00, 0x00, true, 0x00},
        {0x50, 0xF0, false, 0x5F},
        {0x50, 0xB0, true, 0xA0},
        {0xFF, 0xFF, false, 0xFF},
        {0xFF, 0xFF, true, 0x00},
        {0xFF, 0x80, false, 0x7E},
        {0xFF, 0x80, true, 0x7F},
        {0x80, 0xFF, false, 0x80},
        {0x80, 0xFF, true, 0x81}
      })
    );
    uint8_t operation = 0xE9;

    MainBus bus;

    DYNAMIC_SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorInitialValue, amountToSubtract, CarryFlagSet, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      if (CarryFlagSet)
      { 
        uint8_t program[] = {0x38, 0xA9, accumulatorInitialValue, 0xE9, amountToSubtract};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.loadProgram(0x0000, program, n, 0x00);
        bus.cpu.clock();
      }
      else
      {
        uint8_t program[] = {0xA9, accumulatorInitialValue, 0xE9, amountToSubtract};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.loadProgram(0x0000, program, n, 0x00);
      }

      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(accumulatorInitialValue, 2));
      
      bus.cpu.clock();
      REQUIRE(bus.cpu.getRegisterAC() == expectedValue);
    }
  }

  SECTION("SBC Accumulator Correct When In BDC Mode", "[opcode][sbc]")
  {
    auto [accumulatorInitialValue, amountToSubtract, setCarryFlag, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, bool, uint8_t>({
        {0x00, 0x99, false, 0x00},
        {0x00, 0x99, true, 0x01}
      })
    );
    uint8_t operation = 0xE9;

    MainBus bus;

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorInitialValue, amountToSubtract, setCarryFlag, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      if (setCarryFlag)
      { 
        uint8_t program[] = {0x38, 0xF8, 0xA9, accumulatorInitialValue, 0xE9, amountToSubtract};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.loadProgram(0x0000, program, n, 0x00);
        bus.cpu.clock();
      }
      else
      {
        uint8_t program[] = {0xF8, 0xA9, accumulatorInitialValue, 0xE9, amountToSubtract};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.loadProgram(0x0000, program, n, 0x00);
      }

      bus.cpu.clock();
      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(accumulatorInitialValue, 2));
      
      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(expectedValue, 2));
    }
  }

  SECTION("SBC Overflow Correct When Not In BDC Mode")
  {
    auto [accumulatorInitialValue, amountToSubtract, CarryFlagSet, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, bool, uint8_t>({
        {0xFF, 0x01, false, 0},
        {0xFF, 0x00, false, 0},
        {0x80, 0x00, false, 1},
        {0x80, 0x00, true, 0},
        {0x81, 0x01, false, 1},
        {0x81, 0x01, true, 0},
        {0x00, 0x80, false, 0},
        {0x00, 0x80, true, 1},
        {0x01, 0x80, true, 1},
        {0x01, 0x7F, false, 0}
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorInitialValue, amountToSubtract, CarryFlagSet, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      if (CarryFlagSet)
      { 
        uint8_t program[] = {0x38, 0xA9, accumulatorInitialValue, 0xE9, amountToSubtract};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.loadProgram(0x0000, program, n, 0x00);
        bus.cpu.clock();
      }
      else
      {
        uint8_t program[] = {0xA9, accumulatorInitialValue, 0xE9, amountToSubtract};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.loadProgram(0x0000, program, n, 0x00);
      }

      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(accumulatorInitialValue, 2));
      
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.V) == expectedValue);
    }
  }

  SECTION("SBC Overflow Correct When In BDC Mode")
  {
    /// @todo Fix so the commented tests work
    auto [accumulatorInitialValue, amountToSubtract, CarryFlagSet, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, bool, uint8_t>({
        {0x63, 0x01, false, 0},
        {0x63, 0x00, false, 0},
        //{0, 1, false, 1},
        //{1, 1, true, 1},
        //{2, 1, true, 0},
        //{1, 1, false, 0}
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorInitialValue, amountToSubtract, CarryFlagSet, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      if (CarryFlagSet)
      { 
        uint8_t program[] = {0x38, 0xA9, accumulatorInitialValue, 0xE9, amountToSubtract};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.loadProgram(0x0000, program, n, 0x00);
        bus.cpu.clock();
      }
      else
      {
        uint8_t program[] = {0xA9, accumulatorInitialValue, 0xE9, amountToSubtract};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.loadProgram(0x0000, program, n, 0x00);
      }

      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(accumulatorInitialValue, 2));
      
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.V) == expectedValue);
    }
  }

  SECTION("SBC Carry Correct")
  {
    auto [accumulatorInitialValue, amountToSubtract, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t>({
        {0x00, 0x00, 0},
        {0x00, 0x01, 0},
        {0x01, 0x00, 1},
        {0x02, 0x01, 1}
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorInitialValue, amountToSubtract, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      uint8_t program[] = {0xA9, accumulatorInitialValue, 0xE9, amountToSubtract};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(accumulatorInitialValue, 2));
      
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.C) == expectedValue);
    }
  }

  SECTION("SBC Zero Correct")
  {
    auto [accumulatorInitialValue, amountToSubtract, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t>({
        {0x00, 0x00, 0},
        {0x00, 0x01, 0},
        {0x01, 0x00, 1},
        {0x01, 0x01, 0}
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorInitialValue, amountToSubtract, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      uint8_t program[] = {0xA9, accumulatorInitialValue, 0xE9, amountToSubtract};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(accumulatorInitialValue, 2));
      
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedValue);
    }
  }

  SECTION("SBC Negative Correct")
  {
    auto [accumulatorInitialValue, amountToSubtract, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t, uint8_t>({
        {0x80, 0x01, 0},
        {0x81, 0x01, 0},
        {0x00, 0x01, 1},
        {0x01, 0x01, 1}
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorInitialValue, amountToSubtract, expectedValue)
    ) {
      bus.cpu.reset();

      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      uint8_t program[] = {0xA9, accumulatorInitialValue, 0xE9, amountToSubtract};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(accumulatorInitialValue, 2));
      
      bus.cpu.clock();
      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedValue);
    }
  }
}

TEST_CASE("SEC - Set Carry Flag", "[opcode][sec]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x38;

  SECTION("SEC Carry Flag Set Correctly")
  {
    bus.cpu.reset();

    uint8_t program[] = {0x38};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);

    bus.cpu.clock();
    REQUIRE(bus.cpu.getFlag(bus.cpu.C) == 1);
  }
}

TEST_CASE("Set Decimal Mode", "[opcode][sed]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0xF8;

  SECTION("SED Decimal Mode Set Correctly")
  {
    bus.cpu.reset();

    uint8_t program[] = {0xF8};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);

    bus.cpu.clock();
    REQUIRE(bus.cpu.getFlag(bus.cpu.D) == 1);
  }
}

TEST_CASE("SEI - Set Interrupt Flag", "[opcode][sei]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x78;

  SECTION("SEI Interrupt Flag Set Correctly")
  {
    bus.cpu.reset();

    uint8_t program[] = {0x78};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);

    bus.cpu.clock();
    REQUIRE(bus.cpu.getFlag(bus.cpu.I) == 1);
  }
}

TEST_CASE("STA - Store Accumulator In Memory", "[opcode][sta]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x85;

  SECTION("STA Memory Has Correct Value")
  {
    bus.cpu.reset();

    uint8_t program[] = {0xA9, 0x03, 0x85, 0x05};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);

    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(hex(bus.cpuRead(0x05), 2) == hex(0x03, 2));
  }
}

TEST_CASE("STX - Set Memory To X", "[opcode][stx]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x86;

  SECTION("STX Memory Has Correct Value")
  {
    bus.cpu.reset();

    uint8_t program[] = {0xA2, 0x03, 0x86, 0x05};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);

    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(hex(bus.cpuRead(0x05), 2) == hex(0x03, 2));
  }
}

TEST_CASE("STY - Set Memory To Y", "[opcode][sty]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x84;

  SECTION("STY Memory Has Correct Value")
  {
    bus.cpu.reset();

    uint8_t program[] = {0xA0, 0x03, 0x84, 0x05};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);

    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(hex(bus.cpuRead(0x05), 2) == hex(0x03, 2));
  }
}

TEST_CASE("TAX, TAY, TSX, TSY Tests", "[opcode][tax][tay][tsx][tsy]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  //uint8_t operation = 0x0A;

  SECTION("Transfer Correct Value Set")
  {
    auto [sectionOperation, transferFrom, transferTo] = 
      GENERATE( table<uint8_t, ProcessorTests::RegisterMode, ProcessorTests::RegisterMode>({
        {0xAA, ProcessorTests::Accumulator, ProcessorTests::XRegister},
        {0xA8, ProcessorTests::Accumulator, ProcessorTests::YRegister},
        {0x8A, ProcessorTests::XRegister, ProcessorTests::Accumulator},
        {0x98, ProcessorTests::YRegister, ProcessorTests::Accumulator}
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(sectionOperation), transferFrom, transferTo)
    ) {
      bus.cpu.reset();

      uint8_t loadOperation;
      switch (transferFrom)
      {
        case ProcessorTests::Accumulator:
          loadOperation = 0xA9;
          break;
        case ProcessorTests::XRegister:
          loadOperation = 0xA2;
          break;
        case ProcessorTests::YRegister:
          loadOperation = 0xA0;
          break;
      }

      uint8_t program[] = {loadOperation, 0x03, sectionOperation};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      switch (transferFrom)
      {
        case ProcessorTests::Accumulator:
          REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(0x03, 2));
          break;
        case ProcessorTests::XRegister:
          REQUIRE(hex(bus.cpu.getRegisterX(), 2) == hex(0x03, 2));
          break;
        case ProcessorTests::YRegister:
          REQUIRE(hex(bus.cpu.getRegisterY(), 2) == hex(0x03, 2));
          break;
      }
    }
  }

  SECTION("Transfer Negative Value Set")
  {
    MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

    auto [operation, value, transferFrom, expectedResult] = GENERATE( table<uint8_t, uint8_t, ProcessorTests::RegisterMode, uint8_t>({
      {0xAA, 0x80, ProcessorTests::Accumulator, 1},
      {0xA8, 0x80, ProcessorTests::Accumulator, 1},
      {0x8A, 0x80, ProcessorTests::XRegister, 1},
      {0x98, 0x80, ProcessorTests::YRegister, 1},
      {0xAA, 0xFF, ProcessorTests::Accumulator, 1},
      {0xA8, 0xFF, ProcessorTests::Accumulator, 1},
      {0x8A, 0xFF, ProcessorTests::XRegister, 1},
      {0x98, 0xFF, ProcessorTests::YRegister, 1},
      {0xAA, 0x7F, ProcessorTests::Accumulator, 0},
      {0xA8, 0x7F, ProcessorTests::Accumulator, 0},
      {0x8A, 0x7F, ProcessorTests::XRegister, 0},
      {0x98, 0x7F, ProcessorTests::YRegister, 0},
      {0xAA, 0x00, ProcessorTests::Accumulator, 0},
      {0xA8, 0x00, ProcessorTests::Accumulator, 0},
      {0x8A, 0x00, ProcessorTests::XRegister, 0},
      {0x98, 0x00, ProcessorTests::YRegister, 0},
    }));

    MainBus bus;

    DYNAMIC_SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), value, transferFrom, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t loadOperation;
      switch (transferFrom)
      {
        case ProcessorTests::Accumulator:
          loadOperation = 0xA9;
          break;
        case ProcessorTests::XRegister:
          loadOperation = 0xA2;
          break;
        case ProcessorTests::YRegister:
          loadOperation = 0xA0;
          break;
      }

      uint8_t program[] = {loadOperation, value, operation};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedResult);
    }
  }

  SECTION("Transfer Zero Value Set")
  {
    MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

    auto [operation, value, transferFrom, expectedResult] = GENERATE( table<uint8_t, uint8_t, ProcessorTests::RegisterMode, uint8_t>({
      {0xAA, 0xFF, ProcessorTests::Accumulator, 0},
      {0xA8, 0xFF, ProcessorTests::Accumulator, 0},
      {0x8A, 0xFF, ProcessorTests::XRegister, 0},
      {0x98, 0xFF, ProcessorTests::YRegister, 0},
      {0xAA, 0x00, ProcessorTests::Accumulator, 1},
      {0xA8, 0x00, ProcessorTests::Accumulator, 1},
      {0x8A, 0x00, ProcessorTests::XRegister, 1},
      {0x98, 0x00, ProcessorTests::YRegister, 1},
    }));

    MainBus bus;

    DYNAMIC_SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), value, transferFrom, expectedResult)
    ) {
      bus.cpu.reset();

      uint8_t loadOperation;
      switch (transferFrom)
      {
        case ProcessorTests::Accumulator:
          loadOperation = 0xA9;
          break;
        case ProcessorTests::XRegister:
          loadOperation = 0xA2;
          break;
        case ProcessorTests::YRegister:
          loadOperation = 0xA0;
          break;
      }

      uint8_t program[] = {loadOperation, value, operation};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedResult);
    }
  }
}

TEST_CASE("TSX - Transfer Stack Pointer to X Register", "[opcode][tsx]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0xBA;

  SECTION("TSX XRegister Set Correctly")
  {
    bus.cpu.reset();

    uint8_t program[] = {0xBA};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);

    uint8_t stackLocation = bus.cpu.getRegisterSP();

    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(hex(bus.cpu.getRegisterX(), 2) == hex(stackLocation, 2));
  }

  SECTION("TSX Negative Set Correctly")
  {
    auto [valueToLoad, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t>({
        {0x00, 0},
        {0x7F, 0},
        {0x80, 1},
        {0xFF, 1},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), valueToLoad, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA2, valueToLoad, 0x9A, 0xBA};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == expectedValue);
    }
  }

  SECTION("TSX Zero Set Correctly")
  {
    auto [valueToLoad, expectedValue] = 
      GENERATE( table<uint8_t, uint8_t>({
        {0x00, 1},
        {0x01, 0},
        {0xFF, 0},
      })
    );

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), valueToLoad, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA2, valueToLoad, 0x9A, 0xBA};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == expectedValue);
    }
  }
}

TEST_CASE("TXS - Transfer X Register to Stack Pointer", "[opcode][txs]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x9A;

  SECTION("TXS Stack Pointer Set Correctly")
  {
    bus.cpu.reset();

    uint8_t program[] = {0xA2, 0xAA, 0x9A};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x00, program, n, 0x00);

    bus.cpu.clock();
    bus.cpu.clock();

    REQUIRE(hex(bus.cpu.getRegisterSP(), 2) == hex(0xAA, 2));
  }
}

#pragma endregion OPCode

TEST_CASE("Accumulator Address Tests", "[address][acc]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  //uint8_t operation = 0x0A;

  SECTION("Immediate Mode Accumulator Has Correct Result")
  {
    auto [sectionOperation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
      {0x69, 0x01, 0x01, 0x02}, // ADC
      {0x29, 0x03, 0x03, 0x03}, // AND
      {0xA9, 0x04, 0x03, 0x03}, // LDA
      {0x49, 0x55, 0xAA, 0xFF}, // EOR
      {0x09, 0x55, 0xAA, 0xFF}, // ORA
      {0xE9, 0x03, 0x01, 0x01}, // SBC
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(sectionOperation), accumulatorInitialValue, valueToTest, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorInitialValue, sectionOperation, valueToTest};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getRegisterAC() == expectedValue);
    }
  }

  SECTION("ZeroPage Mode Accumulator Has Correct Result")
  {
    auto [sectionOperation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
      {0x65, 0x01, 0x01, 0x02}, // ADC
      {0x25, 0x03, 0x03, 0x03}, // AND
      {0xA5, 0x04, 0x03, 0x03}, // LDA
      {0x45, 0x55, 0xAA, 0xFF}, // EOR
      {0x05, 0x55, 0xAA, 0xFF}, // ORA
      {0xE5, 0x03, 0x01, 0x01}, // SBC
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(sectionOperation), accumulatorInitialValue, valueToTest, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorInitialValue, sectionOperation, 0x05, 0x00, valueToTest};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getRegisterAC() == expectedValue);
    }
  }

  SECTION("ZeroPageX Mode Accumulator Has Correct Result")
  {
    auto [sectionOperation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
      {0x75, 0x00, 0x03, 0x03}, // ADC
      {0x35, 0x03, 0x03, 0x03}, // AND
      {0xB5, 0x04, 0x03, 0x03}, // LDA
      {0x55, 0x55, 0xAA, 0xFF}, // EOR
      {0x15, 0x55, 0xAA, 0xFF}, // ORA
      {0xF5, 0x03, 0x01, 0x01}, // SBC
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(sectionOperation), accumulatorInitialValue, valueToTest, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorInitialValue, 0xA2, 0x01, sectionOperation, 0x06, 0x00, valueToTest};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getRegisterAC() == expectedValue);
    }
  }

  SECTION("Absolute Mode Accumulator Has Correct Result")
  {
    auto [sectionOperation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
      {0x6D, 0x00, 0x03, 0x03}, // ADC
      {0x2D, 0x03, 0x03, 0x03}, // AND
      {0xAD, 0x04, 0x03, 0x03}, // LDA
      {0x4D, 0x55, 0xAA, 0xFF}, // EOR
      {0x0D, 0x55, 0xAA, 0xFF}, // ORA
      {0xED, 0x03, 0x01, 0x01}, // SBC
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(sectionOperation), accumulatorInitialValue, valueToTest, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorInitialValue, sectionOperation, 0x06, 0x00, 0x00, valueToTest};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getRegisterAC() == expectedValue);
    }
  }

  SECTION("AbsoluteX Mode Accumulator Has Correct Result")
  {
    auto [sectionOperation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
      {0x7D, 0x01, 0x01, 0x02}, // ADC
      {0x3D, 0x03, 0x03, 0x03}, // AND
      {0xBD, 0x04, 0x03, 0x03}, // LDA
      {0x5D, 0x55, 0xAA, 0xFF}, // EOR
      {0x1D, 0x55, 0xAA, 0xFF}, // ORA
      {0xFD, 0x03, 0x01, 0x01}, // SBC
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(sectionOperation), accumulatorInitialValue, valueToTest, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[]= {0xA9, accumulatorInitialValue, 0xA2, 0x09, sectionOperation, 0xff, 0xff, 0x00, valueToTest};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getRegisterAC() == expectedValue);
    }
  }

  SECTION("AbsoluteX Mode Accumulator Has Correct Result When Wrapped")
  {
    auto [sectionOperation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
      {0x7D, 0x01, 0x01, 0x02}, // ADC
      {0x3D, 0x03, 0x03, 0x03}, // AND
      {0xBD, 0x04, 0x03, 0x03}, // LDA
      {0x5D, 0x55, 0xAA, 0xFF}, // EOR
      {0x1D, 0x55, 0xAA, 0xFF}, // ORA
      {0xFD, 0x03, 0x01, 0x01}, // SBC
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(sectionOperation), accumulatorInitialValue, valueToTest, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorInitialValue, 0xA2, 0x01, sectionOperation, 0x07, 0x00, 0x00, valueToTest};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getRegisterAC() == expectedValue);
    }
  }

  SECTION("AbsoluteY Mode Accumulator Has Correct Result")
  {
    auto [sectionOperation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
      {0x79, 0x01, 0x01, 0x02}, // ADC
      {0x39, 0x03, 0x03, 0x03}, // AND
      {0xB9, 0x04, 0x03, 0x03}, // LDA
      {0x59, 0x55, 0xAA, 0xFF}, // EOR
      {0x19, 0x55, 0xAA, 0xFF}, // ORA
      {0xF9, 0x03, 0x01, 0x01}, // SBC
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(sectionOperation), accumulatorInitialValue, valueToTest, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[]= {0xA9, accumulatorInitialValue, 0xA0, 0x01, sectionOperation, 0x07, 0x00, 0x00, valueToTest};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(expectedValue, 2));
    }
  }

  SECTION("AbsoluteY Mode Accumulator Has Correct Result When Wrapped")
  {
    auto [sectionOperation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
      {0x79, 0x01, 0x01, 0x02}, // ADC
      {0x39, 0x03, 0x03, 0x03}, // AND
      {0xB9, 0x04, 0x03, 0x03}, // LDA
      {0x59, 0x55, 0xAA, 0xFF}, // EOR
      {0x19, 0x55, 0xAA, 0xFF}, // ORA
      {0xF9, 0x03, 0x01, 0x01}, // SBC
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(sectionOperation), accumulatorInitialValue, valueToTest, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorInitialValue, 0xA0, 0x09, sectionOperation, 0xff, 0xff, 0x00, valueToTest};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(expectedValue, 2));
    }
  }

  SECTION("Indexed Indirect Mode Accumulator Has Correct Result")
  {
    auto [sectionOperation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
      {0x61, 0x01, 0x01, 0x02}, // ADC
      {0x21, 0x03, 0x03, 0x03}, // AND
      {0xA1, 0x04, 0x03, 0x03}, // LDA
      {0x41, 0x55, 0xAA, 0xFF}, // EOR
      {0x01, 0x55, 0xAA, 0xFF}, // ORA
      {0xE1, 0x03, 0x01, 0x01}, // SBC
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(sectionOperation), accumulatorInitialValue, valueToTest, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[]= {0xA9, accumulatorInitialValue, 0xA6, 0x06, sectionOperation, 0x01, 0x06, 0x9, 0x00, valueToTest};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(expectedValue, 2));
    }
  }

  SECTION("Indexed Indirect Mode Accumulator Has Correct Result When Wrapped")
  {
    auto [sectionOperation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
      {0x61, 0x01, 0x01, 0x02}, // ADC
      {0x21, 0x03, 0x03, 0x03}, // AND
      {0xA1, 0x04, 0x03, 0x03}, // LDA
      {0x41, 0x55, 0xAA, 0xFF}, // EOR
      {0x01, 0x55, 0xAA, 0xFF}, // ORA
      {0xE1, 0x03, 0x01, 0x01}, // SBC
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(sectionOperation), accumulatorInitialValue, valueToTest, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorInitialValue, 0xA6, 0x06, sectionOperation, 0xff, 0x08, 0x9, 0x00, valueToTest};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(expectedValue, 2));
    }
  }

  SECTION("Indirect Indexed Mode Accumulator Has Correct Result")
  {
    auto [sectionOperation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
      {0x71, 0x01, 0x01, 0x02}, // ADC
      {0x31, 0x03, 0x03, 0x03}, // AND
      {0xB1, 0x04, 0x03, 0x03}, // LDA
      {0x51, 0x55, 0xAA, 0xFF}, // EOR
      {0x11, 0x55, 0xAA, 0xFF}, // ORA
      {0xF1, 0x03, 0x01, 0x01}, // SBC
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(sectionOperation), accumulatorInitialValue, valueToTest, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[]= {0xA9, accumulatorInitialValue, 0xA0, 0x01, sectionOperation, 0x07, 0x00, 0x08, 0x00, valueToTest};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(expectedValue, 2));
    }
  }

  SECTION("Indirect Indexed Mode Accumulator Has Correct Result When Wrapped")
  {
    auto [sectionOperation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
      {0x71, 0x01, 0x01, 0x02}, // ADC
      {0x31, 0x03, 0x03, 0x03}, // AND
      {0xB1, 0x04, 0x03, 0x03}, // LDA
      {0x51, 0x55, 0xAA, 0xFF}, // EOR
      {0x11, 0x55, 0xAA, 0xFF}, // ORA
      {0xF1, 0x03, 0x01, 0x01}, // SBC
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(sectionOperation), accumulatorInitialValue, valueToTest, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {0xA9, accumulatorInitialValue, 0xA0, 0x0A, sectionOperation, 0x07, 0x00, 0xFF, 0xFF, valueToTest};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(expectedValue, 2));
    }
  }
}

TEST_CASE("Index Address Tests", "[address][index]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x0A;

  SECTION("ZeroPage Mode Index Has Correct Result")
  {
    auto [operation, valueToLoad, testXRegister] = GENERATE( table<uint8_t, uint8_t, bool>({
      {0xA6, 0x03, true},  // LDX Zero Page
      {0xB6, 0x03, true},  // LDX Zero Page Y
      {0xA4, 0x03, false}, // LDY Zero Page
      {0xB4, 0x03, false}, // LDY Zero Page X
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), valueToLoad, testXRegister)
    ) {
      bus.cpu.reset();

      uint8_t program[]= {operation, 0x03, 0x00, valueToLoad};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();

      if (testXRegister)
      {
        REQUIRE(bus.cpu.getRegisterX() == valueToLoad);
      }
      else
      {
        REQUIRE(bus.cpu.getRegisterY() == valueToLoad);
      }
    }
  }

  SECTION("ZeroPageX Mode Index Has Correct Result When Wrapped")
  {
    auto [operation, valueToLoad, testXRegister] = GENERATE( table<uint8_t, uint8_t, bool>({
      {0xB6, 0x03, true},  // LDX Zero Page Y
      {0xB4, 0x03, false}, // LDY Zero Page X
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), valueToLoad, testXRegister)
    ) {
      bus.cpu.reset();

      uint8_t XRegister = testXRegister ? 0xA0 : 0xA2;

      uint8_t program[]= {XRegister, 0xFF, operation, 0x06, 0x00, valueToLoad};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      if (testXRegister)
      {
        REQUIRE(bus.cpu.getRegisterX() == valueToLoad);
      }
      else
      {
        REQUIRE(bus.cpu.getRegisterY() == valueToLoad);
      }
    }
  }

  SECTION("Absolute Mode Index Has Correct Result")
  {
    auto [operation, valueToLoad, testXRegister] = GENERATE( table<uint8_t, uint8_t, bool>({
      {0xAE, 0x03, true},  // LDX Absolute
      {0xAC, 0x03, false}, // LDY Absolute
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), valueToLoad, testXRegister)
    ) {
      bus.cpu.reset();

      uint8_t program[]= {operation, 0x04, 0x00, 0x00, valueToLoad};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();

      if (testXRegister)
      {
        REQUIRE(hex(bus.cpu.getRegisterX(), 2) == hex(valueToLoad, 2));
      }
      else
      {
        REQUIRE(hex(bus.cpu.getRegisterY(), 2) == hex(valueToLoad, 2));
      }
    }
  }
}

TEST_CASE("Compare Address Tests", "[address][compare]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x0A;

  SECTION("Immediate Mode Compare Operation Has Correct Result")
  {
    auto [operation, mode] = GENERATE( table<uint8_t, ProcessorTests::RegisterMode>({
      {0xC9, ProcessorTests::Accumulator}, //CMP Immediate
      {0xE0, ProcessorTests::XRegister}, //CPX Immediate
      {0xC0, ProcessorTests::YRegister} //CPY Immediate
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), mode)
    ) {
      bus.cpu.reset();

      uint8_t loadOperation;
      switch (mode)
      {
        case ProcessorTests::Accumulator:
          loadOperation = 0xA9;
          break;
        case ProcessorTests::XRegister:
          loadOperation = 0xA2;
          break;
        case ProcessorTests::YRegister:
          loadOperation = 0xA0;
          break;
      }

      uint8_t program[]= {loadOperation, 0xFF, operation, 0x00};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == 0);
      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == 1);
      REQUIRE(bus.cpu.getFlag(bus.cpu.C) == 1);
    }
  }

  SECTION("ZeroPage Modes Compare Operation Has Correct Result")
  {
    auto [operation, accumulatorValue, memoryValue, mode] = GENERATE( table<uint8_t, uint8_t, uint8_t, ProcessorTests::RegisterMode>({
      {0xC5, 0xFF, 0x00, ProcessorTests::Accumulator}, //CMP Zero Page
      {0xD5, 0xFF, 0x00, ProcessorTests::Accumulator}, //CMP Zero Page X
      {0xE4, 0xFF, 0x00, ProcessorTests::XRegister}, //CPX Zero Page
      {0xC4, 0xFF, 0x00, ProcessorTests::YRegister}, //CPY Zero Page
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, mode)
    ) {
      bus.cpu.reset();

      uint8_t loadOperation;
      switch (mode)
      {
        case ProcessorTests::Accumulator:
          loadOperation = 0xA9;
          break;
        case ProcessorTests::XRegister:
          loadOperation = 0xA2;
          break;
        case ProcessorTests::YRegister:
          loadOperation = 0xA0;
          break;
      }

      uint8_t program[]= {loadOperation, accumulatorValue, operation, 0x04, memoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == 0);
      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == 1);
      REQUIRE(bus.cpu.getFlag(bus.cpu.C) == 1);
    }
  }

  SECTION("Absolute Modes Compare Operation Has Correct Result")
  {
    auto [operation, accumulatorValue, memoryValue, mode] = GENERATE( table<uint8_t, uint8_t, uint8_t, ProcessorTests::RegisterMode>({
      {0xCD, 0xFF, 0x00, ProcessorTests::Accumulator}, //CMP Absolute
      {0xDD, 0xFF, 0x00, ProcessorTests::Accumulator}, //CMP Absolute X
      {0xEC, 0xFF, 0x00, ProcessorTests::XRegister}, //CPX Absolute
      {0xCC, 0xFF, 0x00, ProcessorTests::YRegister}, //CPY Absolute
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, mode)
    ) {
      bus.cpu.reset();

      uint8_t loadOperation;
      switch (mode)
      {
        case ProcessorTests::Accumulator:
          loadOperation = 0xA9;
          break;
        case ProcessorTests::XRegister:
          loadOperation = 0xA2;
          break;
        case ProcessorTests::YRegister:
          loadOperation = 0xA0;
          break;
      }

      uint8_t program[]= {loadOperation, accumulatorValue, operation, 0x05, 0x00, memoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == 0);
      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == 1);
      REQUIRE(bus.cpu.getFlag(bus.cpu.C) == 1);
    }
  }

  SECTION("Indexed Indirect Mode CMP Operation Has Correct Result")
  {
    auto [operation, accumulatorValue, memoryValue, addressWraps] = GENERATE( table<uint8_t, uint8_t, uint8_t, bool>({
      {0xC1, 0xFF, 0x00, true},
      {0xC1, 0xFF, 0x00, false},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, addressWraps)
    ) {
      bus.cpu.reset();

      std::vector<uint8_t> program;
      if (addressWraps) {
        program = {0xA9, accumulatorValue, 0xA6, 0x06, operation, 0xff, 0x08, 0x9, 0x00, memoryValue};
      } else {
        program = {0xA9, accumulatorValue, 0xA6, 0x06, operation, 0x01, 0x06, 0x9, 0x00, memoryValue};
      }

      bus.cpu.loadProgram(0x0000, program.data(), program.size(), 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == 0);
      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == 1);
      REQUIRE(bus.cpu.getFlag(bus.cpu.C) == 1);
    }
  }

  SECTION("Indirect Indexed Mode CMP Operation Has Correct Result")
  {
    auto [operation, accumulatorValue, memoryValue, addressWraps] = GENERATE( table<uint8_t, uint8_t, uint8_t, bool>({
      {0xD1, 0xFF, 0x00, true},
      {0xD1, 0xFF, 0x00, false},
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), accumulatorValue, memoryValue, addressWraps)
    ) {
      bus.cpu.reset();

      std::vector<uint8_t> program;
      if (addressWraps) {
        program = {0xA9, accumulatorValue, 0x84, 0x06, operation, 0x07, 0x0A, 0xFF, 0xFF, memoryValue};
      } else {
        program = {0xA9, accumulatorValue, 0x84, 0x06, operation, 0x07, 0x01, 0x08, 0x00, memoryValue};
      }

      bus.cpu.loadProgram(0x0000, program.data(), program.size(), 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(bus.cpu.getFlag(bus.cpu.Z) == 0);
      REQUIRE(bus.cpu.getFlag(bus.cpu.N) == 1);
      REQUIRE(bus.cpu.getFlag(bus.cpu.C) == 1);
    }
  }
}

TEST_CASE("Decrement/Increment Address Tests", "[address][inc][dec]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x0A;

  SECTION("Zero Page DEC INC Has Correct Result")
  {
    auto [operation, memoryValue, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t>({
      {0xC6, 0xFF, 0xFE}, //DEC Zero Page
      {0xD6, 0xFF, 0xFE}, //DEC Zero Page X
      {0xE6, 0xFF, 0x00}, //INC Zero Page
      {0xF6, 0xFF, 0x00}, //INC Zero Page X
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), memoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[]= {operation, 0x02, memoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();

      REQUIRE(hex(bus.cpuRead(0x02, true), 2) == hex(expectedValue, 2));
    }
  }

  SECTION("Absolute DEC INC Has Correct Result")
  {
    auto [operation, memoryValue, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t>({
      {0xCE, 0xFF, 0xFE}, //DEC Zero Page
      {0xDE, 0xFF, 0xFE}, //DEC Zero Page X
      {0xEE, 0xFF, 0x00}, //INC Zero Page
      {0xFE, 0xFF, 0x00}, //INC Zero Page X
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, 0x{:02X}) works",
      bus.cpu.executioner.getOperation(operation), memoryValue, expectedValue)
    ) {
      bus.cpu.reset();

      uint8_t program[]= {operation, 0x03, 0x00, memoryValue};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();

      REQUIRE(hex(bus.cpuRead(0x03, true), 2) == hex(expectedValue, 2));
    }
  }
}

TEST_CASE("Store In Memory Address Tests", "[storage][address]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  uint8_t operation = 0x0A;

  SECTION("ZeroPage Mode Memory Has Correct Result")
  {
    auto [operation, mode] = GENERATE( table<uint8_t, ProcessorTests::RegisterMode>({
      {0x85, ProcessorTests::Accumulator}, // STA Zero Page
      {0x95, ProcessorTests::Accumulator}, // STA Zero Page X
      {0x86, ProcessorTests::XRegister}, // STX Zero Page
      {0x96, ProcessorTests::XRegister}, // STX Zero Page Y
      {0x84, ProcessorTests::YRegister}, // STY Zero Page
      {0x94, ProcessorTests::YRegister}, // STY Zero Page X
    }));

    SECTION(
      fmt::format("Check if {:s}({}) works",
      bus.cpu.executioner.getOperation(operation), mode)
    ) {
      bus.cpu.reset();

      uint8_t loadOperation;
      switch (mode)
      {
        case ProcessorTests::Accumulator:
          loadOperation = 0xA9;
          break;
        case ProcessorTests::XRegister:
          loadOperation = 0xA2;
          break;
        case ProcessorTests::YRegister:
          loadOperation = 0xA0;
          break;
      }

      uint8_t program[]= {loadOperation, 0x04, operation, 0x00, 0x05};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(hex(bus.cpuRead(0x04, true), 2) == hex(0x05, 2));
    }
  }

  SECTION("Absolute_Mode_Memory_Has_Correct_Result")
  {
    auto [operation, valueToLoad, mode] = GENERATE( table<uint8_t, uint8_t, ProcessorTests::RegisterMode>({
      {0x8D, 0x03, ProcessorTests::Accumulator}, // STA Absolute
      {0x9D, 0x03, ProcessorTests::Accumulator}, // STA Absolute X
      {0x99, 0x03, ProcessorTests::Accumulator}, // STA Absolute X
      {0x8E, 0x03, ProcessorTests::XRegister}, // STX Zero Page
      {0x8C, 0x03, ProcessorTests::YRegister}, // STY Zero Page
    }));

    SECTION(
      fmt::format("Check if {:s}(0x{:02X}, {}) works",
      bus.cpu.executioner.getOperation(operation), valueToLoad, mode)
    ) {
      bus.cpu.reset();

      uint8_t loadOperation;
      switch (mode)
      {
        case ProcessorTests::Accumulator:
          loadOperation = 0xA9;
          break;
        case ProcessorTests::XRegister:
          loadOperation = 0xA2;
          break;
        case ProcessorTests::YRegister:
          loadOperation = 0xA0;
          break;
      }

      uint8_t program[]= {loadOperation, valueToLoad, operation, 0x04};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      REQUIRE(hex(bus.cpuRead(0x04, true), 2) == hex(valueToLoad, 2));
    }
  }
}

TEST_CASE("Cycle Tests", "[cycle]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;
  //uint8_t operation = 0x0A;

  SECTION("NumberOfCyclesRemaining Correct After Operations That Do Not Wrap")
  {
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
      {0x69, 2}, // ADC Immediate
      {0x65, 3}, // ADC Zero Page
      {0x75, 4}, // ADC Zero Page X
      {0x6D, 4}, // ADC Absolute
      {0x7D, 4}, // ADC Absolute X
      {0x79, 4}, // ADC Absolute Y
      {0x61, 6}, // ADC Indirect X
      {0x71, 5}, // ADC Indirect Y
      {0x29, 2}, // AND Immediate
      {0x25, 3}, // AND Zero Page
      {0x35, 4}, // AND Zero Page X
      {0x2D, 4}, // AND Absolute
      {0x3D, 4}, // AND Absolute X
      {0x39, 4}, // AND Absolute Y
      {0x21, 6}, // AND Indirect X
      {0x31, 5}, // AND Indirect Y
      {0x0A, 2}, // ASL Accumulator
      {0x06, 5}, // ASL Zero Page
      {0x16, 6}, // ASL Zero Page X
      {0x0E, 6}, // ASL Absolute
#if defined EMULATE65C02
      {0x1E, 6}, // ASL Absolute X
#else
      {0x1E, 7}, // ASL Absolute X
#endif
      {0x24, 3}, // BIT Zero Page
      {0x2C, 4}, // BIT Absolute
      {0x00, 7}, // BRK Implied
      {0x18, 2}, // CLC Implied
      {0xD8, 2}, // CLD Implied
      {0x58, 2}, // CLI Implied
      {0xB8, 2}, // CLV Implied
      {0xC9, 2}, // CMP Immediate
      {0xC5, 3}, // CMP ZeroPage
      {0xD5, 4}, // CMP Zero Page X
      {0xCD, 4}, // CMP Absolute
      {0xDD, 4}, // CMP Absolute X
      {0xD9, 4}, // CMP Absolute Y
      {0xC1, 6}, // CMP Indirect X
      {0xD1, 5}, // CMP Indirect Y
      {0xE0, 2}, // CPX Immediate
      {0xE4, 3}, // CPX ZeroPage
      {0xEC, 4}, // CPX Absolute
      {0xC0, 2}, // CPY Immediate
      {0xC4, 3}, // CPY ZeroPage
      {0xCC, 4}, // CPY Absolute
      {0xC6, 5}, // DEC Zero Page
      {0xD6, 6}, // DEC Zero Page X
      {0xCE, 6}, // DEC Absolute
      {0xDE, 7}, // DEC Absolute X
      {0xCA, 2}, // DEX Implied
      {0x88, 2}, // DEY Implied
      {0x49, 2}, // EOR Immediate
      {0x45, 3}, // EOR Zero Page
      {0x55, 4}, // EOR Zero Page X
      {0x4D, 4}, // EOR Absolute
      {0x5D, 4}, // EOR Absolute X
      {0x59, 4}, // EOR Absolute Y
      {0x41, 6}, // EOR Indirect X
      {0x51, 5}, // EOR Indirect Y
      {0xE6, 5}, // INC Zero Page
      {0xF6, 6}, // INC Zero Page X
      {0xEE, 6}, // INC Absolute
      {0xFE, 7}, // INC Absolute X
      {0xE8, 2}, // INX Implied
      {0xC8, 2}, // INY Implied
      {0x4C, 3}, // JMP Absolute
      {0x6C, 5}, // JMP Indirect
      {0x20, 6}, // JSR Absolute
      {0xA9, 2}, // LDA Immediate
      {0xA5, 3}, // LDA Zero Page
      {0xB5, 4}, // LDA Zero Page X
      {0xAD, 4}, // LDA Absolute
      {0xBD, 4}, // LDA Absolute X
      {0xB9, 4}, // LDA Absolute Y
      {0xA1, 6}, // LDA Indirect X
      {0xB1, 5}, // LDA Indirect Y
      {0xA2, 2}, // LDX Immediate
      {0xA6, 3}, // LDX Zero Page
      {0xB6, 4}, // LDX Zero Page Y
      {0xAE, 4}, // LDX Absolute
      {0xBE, 4}, // LDX Absolute Y
      {0xA0, 2}, // LDY Immediate
      {0xA4, 3}, // LDY Zero Page
      {0xB4, 4}, // LDY Zero Page Y
      {0xAC, 4}, // LDY Absolute
      {0xBC, 4}, // LDY Absolute Y
      {0x4A, 2}, // LSR Accumulator
      {0x46, 5}, // LSR Zero Page
      {0x56, 6}, // LSR Zero Page X
      {0x4E, 6}, // LSR Absolute
#if defined EMULATE65C02
      {0x5E, 6}, // LSR Absolute X
#else
      {0x5E, 7}, // LSR Absolute X
#endif
      {0xEA, 2}, // NOP Implied
      {0x09, 2}, // ORA Immediate
      {0x05, 3}, // ORA Zero Page
      {0x15, 4}, // ORA Zero Page X
      {0x0D, 4}, // ORA Absolute
      {0x1D, 4}, // ORA Absolute X
      {0x19, 4}, // ORA Absolute Y
      {0x01, 6}, // ORA Indirect X
      {0x11, 5}, // ORA Indirect Y
      {0x48, 3}, // PHA Implied
      {0x08, 3}, // PHP Implied
      {0x68, 4}, // PLA Implied
      {0x28, 4}, // PLP Implied
      {0x2A, 2}, // ROL Accumulator
      {0x26, 5}, // ROL Zero Page
      {0x36, 6}, // ROL Zero Page X
      {0x2E, 6}, // ROL Absolute
#if defined EMULATE65C02
      {0x3E, 6}, // ROL Absolute X
#else
      {0x3E, 7}, // ROL Absolute X
#endif
      {0x6A, 2}, // ROR Accumulator
      {0x66, 5}, // ROR Zero Page
      {0x76, 6}, // ROR Zero Page X
      {0x6E, 6}, // ROR Absolute
#if defined EMULATE65C02
      {0x7E, 6}, // ROR Absolute X
#else
      {0x7E, 7}, // ROR Absolute X
#endif
      {0x40, 6}, // RTI Implied
      {0x60, 6}, // RTS Implied
      {0xE9, 2}, // SBC Immediate
      {0xE5, 3}, // SBC Zero Page
      {0xF5, 4}, // SBC Zero Page X
      {0xED, 4}, // SBC Absolute
      {0xFD, 4}, // SBC Absolute X
      {0xF9, 4}, // SBC Absolute Y
      {0xE1, 6}, // SBC Indirect X
      {0xF1, 5}, // SBC Indirect Y
      {0x38, 2}, // SEC Implied
      {0xF8, 2}, // SED Implied
      {0x78, 2}, // SEI Implied
      {0x85, 3}, // STA ZeroPage
      {0x95, 4}, // STA Zero Page X
      {0x8D, 4}, // STA Absolute
      {0x9D, 5}, // STA Absolute X
      {0x99, 5}, // STA Absolute Y
      {0x81, 6}, // STA Indirect X
      {0x91, 6}, // STA Indirect Y
      {0x86, 3}, // STX Zero Page
      {0x96, 4}, // STX Zero Page Y
      {0x8E, 4}, // STX Absolute
      {0x84, 3}, // STY Zero Page
      {0x94, 4}, // STY Zero Page X
      {0x8C, 4}, // STY Absolute
      {0xAA, 2}, // TAX Implied
      {0xA8, 2}, // TAY Implied
      {0xBA, 2}, // TSX Implied
      {0x8A, 2}, // TXA Implied
      {0x9A, 2}, // TXS Implied
      {0x98, 2}, // TYA Implied
#if defined ILLEGAL
      {0x4B, 2}, // ALR Immediate
      {0x0B, 2}, // ANC Immediate
      {0x2B, 2}, // ANC2 Immediate
      {0x8B, 2}, // ANE Immediate (Highly Unstable, Magic Constant)
      {0x6B, 2}, // ARR Immediate

      {0xC7, 5}, // DCP Zero Page
      {0xD7, 6}, // DCP Zero Page X
      {0xCF, 6}, // DCP Absolute
      {0xDF, 7}, // DCP Absolute X
#if !defined EMULATE65C02
      {0xDB, 7}, // DCP Absolute Y
#endif
      {0xC3, 8}, // DCP Indirect X
      {0xD3, 8}, // DCP Indirect Y

      {0xE7, 5}, // ISC Zero Page
      {0xF7, 6}, // ISC Zero Page X
      {0xEF, 6}, // ISC Absolute
      {0xFF, 7}, // ISC Absolute X
      {0xFB, 7}, // ISC Absolute Y
      {0xE3, 8}, // ISC Indirect X
      {0xF3, 8}, // ISC Indirect Y

      {0xBB, 4}, // LAS Absolute Y

      {0xA7, 3}, // LAX Zero Page
      {0xB7, 4}, // LAX Zero Page Y
      {0xAF, 4}, // LAX Absolute
      {0xBF, 4}, // LAX Absolute Y
      {0xA3, 6}, // LAX Indirect X
      {0xB3, 5}, // LAX Indirect Y

      {0xAB, 2}, // LXA Immediate (Highly Unstable, Magic Constant)

      {0x67, 5}, // RRA Zero Page
      {0x77, 6}, // RRA Zero Page X
      {0x6F, 6}, // RRA Absolute
      {0x7F, 7}, // RRA Absolute X
      {0x7B, 7}, // RRA Absolute Y
      {0x63, 8}, // RRA Indirect X
      {0x73, 8}, // RRA Indirect Y

      {0x87, 3}, // SAX Zero Page
      {0x97, 4}, // SAX Zero Page Y
      {0x8F, 4}, // SAX Absolute
      {0x83, 6}, // SAX Indirect X

#if !defined EMULATE65C02
      {0xCB, 2}, // SBX Immediate
#endif
      {0x9F, 5}, // SHA Absolute Y (Unstable)
      {0x93, 6}, // SHA Indirect Y (Unstable)

      {0x9E, 5}, // SHX Absolute Y (Unstable)

      {0x9C, 5}, // SHY Absolute X (Unstable)

      {0x07, 5}, // SLO Zero Page
      {0x17, 6}, // SLO Zero Page X
      {0x0F, 6}, // SLO Absolute
      {0x1F, 7}, // SLO Absolute X
      {0x1B, 7}, // SLO Absolute Y
      {0x03, 8}, // SLO Indirect X
      {0x13, 8}, // SLO Indirect Y

      {0x47, 5}, // SRE Zero Page
      {0x57, 6}, // SRE Zero Page X
      {0x4F, 6}, // SRE Absolute
      {0x5F, 7}, // SRE Absolute X
      {0x5B, 7}, // SRE Absolute Y
      {0x43, 8}, // SRE Indirect X
      {0x53, 8}, // SRE Indirect Y

      {0x9B, 5}, // TAS Absolute Y

      {0xEB, 2}, // USBC Immediate
#endif
    }));

    SECTION(
      fmt::format("Check if {:s}({}) works",
      bus.cpu.executioner.getOperation(operation), numberOfCyclesUsed)
    ) {
      bus.cpu.reset();

      uint8_t program[] = {operation, 0x00};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      uint8_t startingNumberOfCycles = bus.cpu.getOperationCycleCount();

      bus.cpu.clock();

      REQUIRE(hex(startingNumberOfCycles + numberOfCyclesUsed, 4) == hex(bus.cpu.getOperationCycleCount(), 4));
    }
  }

  SECTION("NumberOfCyclesRemaining Correct When In AbsoluteX And Wrap")
  {
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
      {0x7D, 5}, // ADC Absolute X
      {0x3D, 5}, // AND Absolute X
      {0x1E, 7}, // ASL Absolute X
      {0xDD, 5}, // CMP Absolute X
      {0xDE, 7}, // DEC Absolute X
      {0x5D, 5}, // EOR Absolute X
      {0xFE, 7}, // INC Absolute X
      {0xBD, 5}, // LDA Absolute X
      {0xBC, 5}, // LDY Absolute X
      {0x5E, 7}, // LSR Absolute X
      {0x1D, 5}, // ORA Absolute X
      {0x3E, 7}, // ROL Absolute X
      {0x7E, 7}, // ROR Absolute X
      {0xFD, 5}, // SBC Absolute X
      {0x9D, 5}, // STA Absolute X
      {0x99, 5}, // STA Absolute Y
    }));

    SECTION(
      fmt::format("Check if {:s}({:}) works",
      bus.cpu.executioner.getOperation(operation), numberOfCyclesUsed)
    ) {
      bus.cpu.reset();

      uint8_t program[]= {0xA6, 0x06, operation, 0xff, 0xff, 0x00, 0x03};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();

      //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
      uint8_t startingNumberOfCycles = bus.cpu.getOperationCycleCount();
      bus.cpu.clock();

      REQUIRE(hex(startingNumberOfCycles + numberOfCyclesUsed, 4) == hex(bus.cpu.getOperationCycleCount(), 4));
    }
  }

  SECTION("NumberOfCyclesRemaining Correct When In AbsoluteY And Wrap")
  {
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
      {0x79, 5}, // ADC Absolute Y
      {0x39, 5}, // AND Absolute Y
      {0xD9, 5}, // CMP Absolute Y
      {0x59, 5}, // EOR Absolute Y
      {0xB9, 5}, // LDA Absolute Y
      {0xBE, 5}, // LDX Absolute Y
      {0x19, 5}, // ORA Absolute Y
      {0xF9, 5}, // SBC Absolute Y
    }));

    SECTION(
      fmt::format("Check if {:s}({:}) works",
      bus.cpu.executioner.getOperation(operation), numberOfCyclesUsed)
    ) {
      bus.cpu.reset();

      uint8_t program[]= {0xA4, 0x06, operation, 0xff, 0xff, 0x00, 0x03};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      bus.cpu.clock();

      //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
      uint8_t startingNumberOfCycles = bus.cpu.getOperationCycleCount();
      bus.cpu.clock();

      REQUIRE(hex(startingNumberOfCycles + numberOfCyclesUsed, 4) == hex(bus.cpu.getOperationCycleCount(), 4));
    }
  }

  SECTION("NumberOfCyclesRemaining Correct When In IndirectIndexed And Wrap")
  {
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
      {0x71, 6}, // ADC Indirect Y
      {0x31, 6}, // AND Indirect Y
      {0xB1, 6}, // LDA Indirect Y
      {0xD1, 6}, // CMP Indirect Y
      {0x51, 6}, // EOR Indirect Y
      {0x11, 6}, // ORA Indirect Y
      {0xF1, 6}, // SBC Indirect Y
      {0x91, 6}, // STA Indirect Y
    }));

    SECTION(
      fmt::format("Check if {:s}({:}) works",
      bus.cpu.executioner.getOperation(operation), numberOfCyclesUsed)
    ) {
      bus.cpu.reset();

      uint8_t program[]= {0xA0, 0x04, operation, 0x05, 0x08, 0xFF, 0xFF, 0x03};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      bus.cpu.clock();

      //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
      uint8_t startingNumberOfCycles = bus.cpu.getOperationCycleCount();
      bus.cpu.clock();

      REQUIRE(hex(startingNumberOfCycles + numberOfCyclesUsed, 4) == hex(bus.cpu.getOperationCycleCount(), 4));
    }
  }

  SECTION("NumberOfCyclesRemaining Correct When Relative And Branch On Carry Set")
  {
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
      {0x90, 2}, // BCC
      {0xB0, 3}, // BCS
    }));

    SECTION(
      fmt::format("Check if {:s}({:}) works",
      bus.cpu.executioner.getOperation(operation), numberOfCyclesUsed)
    ) {
      bus.cpu.reset();

      uint8_t program[]= {0x38, operation, 0x00};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(0x00, 2));

      bus.cpu.clock();

      //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
      uint8_t startingNumberOfCycles = bus.cpu.getOperationCycleCount();
      bus.cpu.clock();

      REQUIRE(hex(startingNumberOfCycles + numberOfCyclesUsed, 4) == hex(bus.cpu.getOperationCycleCount(), 4));
    }
  }

  SECTION("NumberOfCyclesRemaining Correct When Relative And Branch On Carry Clear")
  {
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
      {0x90, 3}, // BCC
      {0xB0, 2}, // BCS
    }));

    SECTION(
      fmt::format("Check if {:s}({:}) works",
      bus.cpu.executioner.getOperation(operation), numberOfCyclesUsed)
    ) {
      bus.cpu.reset();

      uint8_t program[]= {0x18, operation, 0x00};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(0x00, 2));

      bus.cpu.clock();

      //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
      uint8_t startingNumberOfCycles = bus.cpu.getOperationCycleCount();
      bus.cpu.clock();

      REQUIRE(hex(startingNumberOfCycles + numberOfCyclesUsed, 4) == hex(bus.cpu.getOperationCycleCount(), 4));
    }
  }

  SECTION("NumberOfCyclesRemaining Correct When Relative And Branch On Carry And Wrap")
  {
    auto [operation, numberOfCyclesUsed, isCarrySet, wrapRight] = GENERATE( table<uint8_t, uint8_t, bool, bool>({
      {0x90, 4, false, true},  //BCC
      {0x90, 4, false, false}, //BCC
      {0xB0, 4, true, true},   //BCC
      {0xB0, 4, true, false},  //BCC
    }));

    SECTION(
      fmt::format("Check if {:s}({:d}, {}, {}) works",
      bus.cpu.executioner.getOperation(operation), numberOfCyclesUsed, isCarrySet, wrapRight)
    ) {
      bus.cpu.reset();

      uint8_t carryOperation = isCarrySet ? 0x38 : 0x18;
      uint16_t initialAddress = wrapRight ? 0xFFF0 : 0x00;
      uint8_t amountToMove = wrapRight ? 0x0F : 0x84;

      uint8_t program[]= {carryOperation, operation, amountToMove, 0x00};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(initialAddress, program, n, initialAddress);
      REQUIRE(hex(bus.cpu.getRegisterAC(), 2) == hex(0x00, 2));

      bus.cpu.clock();

      //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
      uint8_t startingNumberOfCycles = bus.cpu.getOperationCycleCount();
      bus.cpu.clock();

      REQUIRE(hex(startingNumberOfCycles + numberOfCyclesUsed, 4) == hex(bus.cpu.getOperationCycleCount(), 4));
    }
  }

  SECTION("NumberOfCyclesRemaining Correct When Relative And Branch On Zero Set")
  {
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
      {0xF0, 3}, // BEQ
      {0xD0, 2}, // BNE
    }));

    SECTION(
      fmt::format("Check if {:s}({:}) works",
      bus.cpu.executioner.getOperation(operation), numberOfCyclesUsed)
    ) {
      bus.cpu.reset();

      uint8_t program[]= {0xA9, 0x00, operation, 0x00};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();

      //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
      uint8_t startingNumberOfCycles = bus.cpu.getOperationCycleCount();
      bus.cpu.clock();

      REQUIRE(hex(startingNumberOfCycles + numberOfCyclesUsed, 4) == hex(bus.cpu.getOperationCycleCount(), 4));
    }
  }

  SECTION("NumberOfCyclesRemaining Correct When Relative And Branch On Zero Clear")
  {
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
      {0x90, 3}, // BEQ
      {0xB0, 2}, // BNE
    }));

    SECTION(
      fmt::format("Check if {:s}({:}) works",
      bus.cpu.executioner.getOperation(operation), numberOfCyclesUsed)
    ) {
      bus.cpu.reset();

      uint8_t program[]= {0xA9, 0x01, operation, 0x00};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();

      //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
      uint8_t startingNumberOfCycles = bus.cpu.getOperationCycleCount();
      bus.cpu.clock();

      REQUIRE(hex(startingNumberOfCycles + numberOfCyclesUsed, 4) == hex(bus.cpu.getOperationCycleCount(), 4));
    }
  }

  SECTION("NumberOfCyclesRemaining Correct When Relative And Branch On Zero And Wrap")
  {
    auto [operation, numberOfCyclesUsed, isZeroSet, wrapRight] = GENERATE( table<uint8_t, uint8_t, bool, bool>({
      {0xF0, 4, true, true},  //BEQ
      {0xF0, 4, true, false}, //BEQ
      {0xD0, 4, false, true},   //BNE
      {0xD0, 4, false, false},  //BNE
    }));

    SECTION(
      fmt::format("Check if {:s}({:d}, {}, {}) works",
      bus.cpu.executioner.getOperation(operation), numberOfCyclesUsed, isZeroSet, wrapRight)
    ) {
      bus.cpu.reset();

      uint8_t newAccumulatorValue = isZeroSet ? 0x00 : 0x01;
      uint16_t initialAddress = wrapRight ? 0xFFF0 : 0x00;
      uint8_t amountToMove = wrapRight ? 0x0D : 0x84;

      uint8_t program[]= {0xA9, newAccumulatorValue, operation, amountToMove, 0x00};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(initialAddress, program, n, initialAddress);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();

      //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
      uint8_t startingNumberOfCycles = bus.cpu.getOperationCycleCount();
      bus.cpu.clock();

      REQUIRE(hex(startingNumberOfCycles + numberOfCyclesUsed, 4) == hex(bus.cpu.getOperationCycleCount(), 4));
    }
  }

  SECTION("NumberOfCyclesRemaining Correct When Relative And Branch On Negative Set")
  {
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
      {0x30, 3}, // BEQ
      {0x10, 2}, // BNE
    }));

    SECTION(
      fmt::format("Check if {:s}({:}) works",
      bus.cpu.executioner.getOperation(operation), numberOfCyclesUsed)
    ) {
      bus.cpu.reset();

      uint8_t program[]= {0xA9, 0x80, operation, 0x00};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();

      //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
      uint8_t startingNumberOfCycles = bus.cpu.getOperationCycleCount();
      bus.cpu.clock();

      REQUIRE(hex(startingNumberOfCycles + numberOfCyclesUsed, 4) == hex(bus.cpu.getOperationCycleCount(), 4));
    }
  }

  SECTION("NumberOfCyclesRemaining Correct When Relative And Branch On Negative Clear")
  {
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
      {0x30, 2}, // BEQ
      {0x10, 3}, // BNE
    }));

    SECTION(
      fmt::format("Check if {:s}({:}) works",
      bus.cpu.executioner.getOperation(operation), numberOfCyclesUsed)
    ) {
      bus.cpu.reset();

      uint8_t program[]= {0xA9, 0x79, operation, 0x00};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();

      //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
      uint8_t startingNumberOfCycles = bus.cpu.getOperationCycleCount();
      bus.cpu.clock();

      REQUIRE(hex(startingNumberOfCycles + numberOfCyclesUsed, 4) == hex(bus.cpu.getOperationCycleCount(), 4));
    }
  }

  SECTION("NumberOfCyclesRemaining Correct When Relative And Branch On Negative And Wrap")
  {
    auto [operation, numberOfCyclesUsed, isNegativeSet, wrapRight] = GENERATE( table<uint8_t, uint8_t, bool, bool>({
      {0x30, 4, true, true},  //BEQ
      {0x30, 4, true, false}, //BEQ
      {0x10, 4, false, true},   //BNE
      {0x10, 4, false, false},  //BNE
    }));

    SECTION(
      fmt::format("Check if {:s}({:d}, {}, {}) works",
      bus.cpu.executioner.getOperation(operation), numberOfCyclesUsed, isNegativeSet, wrapRight)
    ) {
      bus.cpu.reset();

      uint8_t newAccumulatorValue = isNegativeSet ? 0x80 : 0x79;
      uint16_t initialAddress = wrapRight ? 0xFFF0 : 0x00;
      uint8_t amountToMove = wrapRight ? 0x0D : 0x84;

      uint8_t program[]= {0xA9, newAccumulatorValue, operation, amountToMove, 0x00};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(initialAddress, program, n, initialAddress);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);
      bus.cpu.clock();

      //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
      uint8_t startingNumberOfCycles = bus.cpu.getOperationCycleCount();
      bus.cpu.clock();

      REQUIRE(hex(startingNumberOfCycles + numberOfCyclesUsed, 4) == hex(bus.cpu.getOperationCycleCount(), 4));
    }
  }

  SECTION("NumberOfCyclesRemaining Correct When Relative And Branch On Overflow Set")
  {
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
      {0x50, 2}, // BVC
      {0x70, 3}, // BVS
    }));

    SECTION(
      fmt::format("Check if {:s}({:d}) works",
      bus.cpu.executioner.getOperation(operation), numberOfCyclesUsed)
    ) {
      bus.cpu.reset();

      uint8_t program[]= {0xA9, 0x01, 0x69, 0x7F, operation, 0x00};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
      uint8_t startingNumberOfCycles = bus.cpu.getOperationCycleCount();
      bus.cpu.clock();

      REQUIRE(hex(startingNumberOfCycles + numberOfCyclesUsed, 4) == hex(bus.cpu.getOperationCycleCount(), 4));
    }
  }

  SECTION("NumberOfCyclesRemaining Correct When Relative And Branch On Overflow Clear")
  {
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
      {0x50, 3}, // BVC
      {0x70, 2}, // BVS
    }));

    SECTION(
      fmt::format("Check if {:s}({:d}) works",
      bus.cpu.executioner.getOperation(operation), numberOfCyclesUsed)
    ) {
      bus.cpu.reset();

      uint8_t program[]= {0xA9, 0x01, 0x69, 0x01, operation, 0x00};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);
      REQUIRE(bus.cpu.getRegisterAC() == 0x00);

      bus.cpu.clock();
      bus.cpu.clock();

      //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
      uint8_t startingNumberOfCycles = bus.cpu.getOperationCycleCount();
      bus.cpu.clock();

      REQUIRE(hex(startingNumberOfCycles + numberOfCyclesUsed, 4) == hex(bus.cpu.getOperationCycleCount(), 4));
    }
  }

  SECTION("NumberOfCyclesRemaining Correct When Relative And Branch On Overflow And Wrap")
  {
    auto [operation, numberOfCyclesUsed, isOverflowSet, wrapRight] = GENERATE( table<uint8_t, uint8_t, bool, bool>({
      {0x50, 4, false, true},  //BVC
      {0x50, 4, false, false}, //BVC
      {0x70, 4, true, true},   //BVS
      {0x70, 4, true, false},  //BVS
    }));

    SECTION(
      fmt::format("Check if {:s}({:d}, {}, {}) works",
      bus.cpu.executioner.getOperation(operation), numberOfCyclesUsed, isOverflowSet, wrapRight)
    ) {
      bus.cpu.reset();

      uint8_t newAccumulatorValue = isOverflowSet ? 0x7F : 0x00;
      uint16_t initialAddress = wrapRight ? 0xFFF0 : 0x00;
      uint8_t amountToMove = wrapRight ? 0x0B : 0x86;

      uint8_t program[]= {0xA9, newAccumulatorValue, 0x69, 0x01, operation, amountToMove, 0x00};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(initialAddress, program, n, initialAddress);

      bus.cpu.clock();
      bus.cpu.clock();

      //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
      uint8_t startingNumberOfCycles = bus.cpu.getOperationCycleCount();
      bus.cpu.clock();

      REQUIRE(hex(startingNumberOfCycles + numberOfCyclesUsed, 4) == hex(bus.cpu.getOperationCycleCount(), 4));
    }
  }

  SECTION("NumberOfCyclesRemaining Correct After NOP Operations")
  {
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
      {0xEA, 2}, // NOP Implied
#if defined ILLEGAL
      {0x1A, 2}, // NOP Implied
      {0x3A, 2}, // NOP Implied
      {0x5A, 2}, // NOP Implied
      {0x7A, 2}, // NOP Implied
      {0xDA, 2}, // NOP Implied
      {0xFA, 2}, // NOP Implied
      {0x80, 2}, // DOP Immediate
      {0x82, 2}, // DOP Immediate
      {0x89, 2}, // DOP Immediate
      {0xC2, 2}, // DOP Immediate
      {0xE2, 2}, // DOP Immediate
      {0x0C, 4}, // TOP Absolute
      {0x1C, 4}, // TOP AbsoluteX
      {0x3C, 4}, // TOP AbsoluteX
      {0x5C, 4}, // TOP AbsoluteX
      {0x7C, 4}, // TOP AbsoluteX
      {0xDC, 4}, // TOP AbsoluteX
      {0xFC, 4}, // TOP AbsoluteX
      {0x04, 3}, // DOP ZeroPage
      {0x44, 3}, // DOP ZeroPage
      {0x64, 3}, // DOP ZeroPage
      {0x14, 4}, // DOP ZeroPageX
      {0x34, 4}, // DOP ZeroPageX
      {0x54, 4}, // DOP ZeroPageX
      {0x74, 4}, // DOP ZeroPageX
      {0xD4, 4}, // DOP ZeroPageX
      {0xF4, 4}, // DOP ZeroPageX
#endif
    }));

    SECTION(
      fmt::format("Check if {:s}({:d}) works",
      bus.cpu.executioner.getOperation(operation), numberOfCyclesUsed)
    ) {
      bus.cpu.reset();

      uint8_t program[]= {operation, 0x00};
      size_t n = sizeof(program) / sizeof(program[0]);
      bus.cpu.loadProgram(0x0000, program, n, 0x00);

      uint8_t startingNumberOfCycles = bus.cpu.getOperationCycleCount();

      bus.cpu.clock();

      REQUIRE(hex(startingNumberOfCycles + numberOfCyclesUsed, 4) == hex(bus.cpu.getOperationCycleCount(), 4));
    }
  }
}

TEST_CASE("Program Counter Tests", "[programcounter][pc]")
{
  MainTest::logTestCaseName(Catch::getResultCapture().getCurrentTestName());

  MainBus bus;

  bus.cpu.reset();
  REQUIRE(hex(bus.cpu.getProgramCounter(), 4) == hex(0x0000, 4));

  SECTION("Branch On Negative Set Program Counter Correct When NoBranch Occurs")
  {
    uint8_t program[]= {0xA9, 0x80, 0x10};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);
    REQUIRE(bus.cpu.getRegisterAC() == 0x00);

    //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
    bus.cpu.clock();
    uint16_t currentProgramCounter = bus.cpu.getProgramCounter();
    bus.cpu.clock();

    REQUIRE(hex(currentProgramCounter + 2, 4) == hex(bus.cpu.getProgramCounter(), 4));
  }

  SECTION("Branch On Negative Clear Program Counter Correct When NoBranch Occurs")
  {
    uint8_t program[]= {0xA9, 0x79, 0x30};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);
    REQUIRE(bus.cpu.getRegisterAC() == 0x00);

    //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
    bus.cpu.clock();
    uint16_t currentProgramCounter = bus.cpu.getProgramCounter();
    bus.cpu.clock();

    REQUIRE(hex(currentProgramCounter + 2, 4) == hex(bus.cpu.getProgramCounter(), 4));
  }

  SECTION("Branch On Overflow Set Program Counter Correct When NoBranch Occurs")
  {
    uint8_t program[]= {0xA9, 0x01, 0x69, 0x01, 0x70, 0x00};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);
    REQUIRE(bus.cpu.getProgramCounter() == 0x0000);

    //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
    bus.cpu.clock();
    bus.cpu.clock();
    uint16_t currentProgramCounter = bus.cpu.getProgramCounter();

    bus.cpu.clock();

    REQUIRE(hex(currentProgramCounter + 0x2, 4) == hex(bus.cpu.getProgramCounter(), 4));
  }

  SECTION("Branch On Overflow Clear Program Counter Correct When NoBranch Occurs")
  {
    uint8_t program[]= {0xA9, 0x01, 0x69, 0x7F, 0x50, 0x00};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0x0000, program, n, 0x00);
    REQUIRE(bus.cpu.getProgramCounter() == 0x0000);

    // Get the number of cycles after the register has been
    // loaded, so we can isolate the operation under test
    bus.cpu.clock();
    bus.cpu.clock();
    uint16_t currentProgramCounter = bus.cpu.getProgramCounter();

    bus.cpu.clock();

    REQUIRE(hex(currentProgramCounter + 0x2, 4) == hex(bus.cpu.getProgramCounter(), 4));
  }

  SECTION("Program Counter Wraps Correctly")
  {
    uint8_t program[] = {0x38};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.loadProgram(0xFFFF, program, n, 0xFFFF);

    REQUIRE(hex(bus.cpu.getProgramCounter(), 4) == hex(0xFFFF, 4));
    bus.cpu.clock();

    REQUIRE(hex(bus.cpu.getProgramCounter(), 4) == hex(0x0000, 4));
  }
}
};
