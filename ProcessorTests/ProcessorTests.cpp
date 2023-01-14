#include <catch2/catch_all.hpp>
#include <iostream>

#include "Bus.hpp"
#include "Processor.hpp"
#include "Common.hpp"
#include "ProcessorTests.hpp"

using namespace CPU;
using namespace CPUTest;

Bus bus;

#pragma region Initialization Tests
TEST_CASE("Processor Status Flags Initialized Correctly", "[init]")
{
    /*
        C = Carry Bit
        Z = Zero
        I = Disable Interrupts
        D = Decimal Mode (unused in this implementation)
        B = Break
        U = Unused
        V = Overflow
        N = Negative
    */
    //Bus bus;

    REQUIRE(bus.cpu.GetFlag(Processor::FLAGS6502::C) == 0x00);
    REQUIRE(bus.cpu.GetFlag(Processor::FLAGS6502::Z) == 0x00);
    REQUIRE(bus.cpu.GetFlag(Processor::FLAGS6502::I) == 0x00);
    REQUIRE(bus.cpu.GetFlag(Processor::FLAGS6502::D) == 0x00);
    REQUIRE(bus.cpu.GetFlag(Processor::FLAGS6502::B) == 0x00);
    REQUIRE(bus.cpu.GetFlag(Processor::FLAGS6502::U) == 0x00);
    REQUIRE(bus.cpu.GetFlag(Processor::FLAGS6502::V) == 0x00);
    REQUIRE(bus.cpu.GetFlag(Processor::FLAGS6502::N) == 0x00);
}

TEST_CASE("Processor Registers Initialized Correctly", "[init]")
{
    //Bus bus;

    REQUIRE(bus.cpu.a == 0x00);
    REQUIRE(bus.cpu.x == 0x00);
    REQUIRE(bus.cpu.y == 0x00);
    REQUIRE(bus.cpu.opcode == 0x00);
    REQUIRE(bus.cpu.pc == 0x00);
}

TEST_CASE("ProgramCounter Correct When Program Loaded", "[init]")
{
    //Bus bus;

    uint8_t program[] = {0x01};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x01);
    REQUIRE(bus.cpu.pc == 0x01);
}

TEST_CASE("Throws Exception When OpCode Is Invalid", "[init][ILLEGAL]")
{
    //Bus bus;

    uint8_t program[] = {0xFF};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);
    //REQUIRE(bus.cpu.tick() == 0x00);
}

TEST_CASE("Stack Pointer Initializes To Default Value After Reset", "[init]")
{
    //Bus bus;

    bus.cpu.reset();

    REQUIRE(bus.cpu.stkp == 0xFD);
}
#pragma endregion Initialization Tests

#pragma region ADC - Add with Carry Tests
TEST_CASE("ADC Accumulator Correct When Not In BDC Mode", "[opcode][adc]")
{
    auto [accumlatorInitialValue, amountToAdd, CarryFlagSet, expectedValue] = 
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

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(" << accumlatorInitialValue << ", " << amountToAdd << ", " << CarryFlagSet << ", " << expectedValue << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        if (CarryFlagSet)
        { 
            uint8_t program[] = {0x38, 0xA9, accumlatorInitialValue, 0x69, amountToAdd};
            size_t n = sizeof(program) / sizeof(program[0]);
            bus.cpu.LoadProgram(0x0000, program, n, 0x00);
            bus.cpu.tick();
        }
        else
        {
            uint8_t program[] = {0xA9, accumlatorInitialValue, 0x69, amountToAdd};
            size_t n = sizeof(program) / sizeof(program[0]);
            bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        }

        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.a, 2) == hex(accumlatorInitialValue, 2));
        
        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.a, 2) == hex(expectedValue, 2));
    }
}

TEST_CASE("ADC Accumulator Correct When In BDC Mode", "[opcode][adc]")
{
    auto [accumlatorInitialValue, amountToAdd, CarryFlagSet, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, bool, uint8_t>({
            {0x99, 0x99, false, 0x98},
            {0x99, 0x99, true, 0x99},
            {0x90, 0x99, false, 0x89}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(" << accumlatorInitialValue << ", " << amountToAdd << ", " << CarryFlagSet << ", " << expectedValue << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        if (CarryFlagSet)
        { 
            uint8_t program[] = {0x38, 0xF8, 0xA9, accumlatorInitialValue, 0x69, amountToAdd};
            size_t n = sizeof(program) / sizeof(program[0]);
            bus.cpu.LoadProgram(0x0000, program, n, 0x00);
            bus.cpu.tick();
        }
        else
        {
            uint8_t program[] = {0xF8, 0xA9, accumlatorInitialValue, 0x69, amountToAdd};
            size_t n = sizeof(program) / sizeof(program[0]);
            bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        }

        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.a, 2) == hex(accumlatorInitialValue, 2));
        
        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.a, 2) == hex(expectedValue, 2));
    }
}

TEST_CASE("ADC Carry Correct When Not In BDC Mode", "[opcode][adc]")
{
    auto [accumlatorInitialValue, amountToAdd, CarryFlagSet, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, bool, bool>({
            {0xFE, 0x01, false, false},
            {0xFE, 0x01, true, true},
            {0xFD, 0x01, true, false},
            {0xFF, 0x01, false, true},
            {0xFF, 0x01, true, true}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(" << accumlatorInitialValue << ", " << amountToAdd << ", " << CarryFlagSet << ", " << expectedValue << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        if (CarryFlagSet)
        { 
            uint8_t program[] = {0x38, 0xA9, accumlatorInitialValue, 0x69, amountToAdd};
            size_t n = sizeof(program) / sizeof(program[0]);
            bus.cpu.LoadProgram(0x0000, program, n, 0x00);
            bus.cpu.tick();
        }
        else
        {
            uint8_t program[] = {0xA9, accumlatorInitialValue, 0x69, amountToAdd};
            size_t n = sizeof(program) / sizeof(program[0]);
            bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        }

        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.a, 2) == hex(accumlatorInitialValue, 2));
        
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.C) == 1) == expectedValue);
    }
}

TEST_CASE("ADC Carry Correct When In BDC Mode", "[opcode][adc]")
{
    auto [accumlatorInitialValue, amountToAdd, CarryFlagSet, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, bool, bool>({
            {0x62, 0x01, false, false},
            {0x62, 0x01, true, false},
            {0x63, 0x01, false, false},
            {0x63, 0x01, true, false}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(" << accumlatorInitialValue << ", " << amountToAdd << ", " << CarryFlagSet << ", " << expectedValue << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        uint8_t program[] = {0xF8, 0xA9, accumlatorInitialValue, 0x69, amountToAdd};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.a, 2) == hex(accumlatorInitialValue, 2));
        
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.C) == 1) == expectedValue);
    }
}

TEST_CASE("ADC Zero Flag Correct When Not In BDC Mode", "[opcode][adc]")
{
    auto [accumlatorInitialValue, amountToAdd, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, bool>({
            {0x00, 0x00, true},
            {0xFF, 0x01, true},
            {0x00, 0x01, false},
            {0x01, 0x00, false}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(" << accumlatorInitialValue << ", " << amountToAdd << ", " << expectedValue << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        uint8_t program[] = {0xA9, accumlatorInitialValue, 0x69, amountToAdd};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.a, 2) == hex(accumlatorInitialValue, 2));
        
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.Z) == 1) == expectedValue);
    }
}

TEST_CASE("ADC Negative Flag Correct", "[opcode][adc]")
{
    auto [accumlatorInitialValue, amountToAdd, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, bool>({
            {0x7E, 0x01, false},
            {0x01, 0x7E, false},
            {0x01, 0x7F, true},
            {0x7F, 0x01, true},
            {0x01, 0xFE, true},
            {0xFE, 0x01, true},
            {0x01, 0xFF, false},
            {0xFF, 0x01, false},
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(" << accumlatorInitialValue << ", " << amountToAdd << ", " << expectedValue << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        uint8_t program[] = {0xA9, accumlatorInitialValue, 0x69, amountToAdd};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.a, 2) == hex(accumlatorInitialValue, 2));
        
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.N) == 1) == expectedValue);
    }
}

TEST_CASE("ADC Overflow Flag Correct", "[opcode][adc]")
{
    auto [accumlatorInitialValue, amountToAdd, CarryFlagSet, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, bool, bool>({
            {0x00, 0x7F, false, false},
            {0x00, 0x80, false, false},
            {0x01, 0x7F, false, true},
            {0x01, 0x80, false, false},
            {0x7F, 0x01, false, true},
            {0x7F, 0x7F, false, true},
            {0x80, 0x7F, false, false},
            {0x80, 0x80, false, true},
            {0x80, 0x81, false, true},
            {0x80, 0xFF, false, true},
            {0xFF, 0x00, false, false},
            {0xFF, 0x01, false, false},
            {0xFF, 0x7F, false, false},
            {0xFF, 0x80, false, true},
            {0xFF, 0xFF, false, false},
            {0x00, 0x7F, true, true},
            {0x00, 0x80, true, false},
            {0x01, 0x7F, true, true},
            {0x01, 0x80, true, false},
            {0x7F, 0x01, true, true},
            {0x7F, 0x7F, true, true},
            {0x80, 0x7F, true, false},
            {0x80, 0x80, true, true},
            {0x80, 0x81, true, true},
            {0x80, 0xFF, true, false},
            {0xFF, 0x00, true, false},
            {0xFF, 0x01, true, false},
            {0xFF, 0x7F, true, false},
            {0xFF, 0x80, true, false},
            {0xFF, 0xFF, true, false}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(" << accumlatorInitialValue << ", " << amountToAdd << ", " << CarryFlagSet << ", " << expectedValue << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        if (CarryFlagSet)
        { 
            uint8_t program[] = {0x38, 0xA9, accumlatorInitialValue, 0x69, amountToAdd};
            size_t n = sizeof(program) / sizeof(program[0]);
            bus.cpu.LoadProgram(0x0000, program, n, 0x00);
            bus.cpu.tick();
        }
        else
        {
            uint8_t program[] = {0xA9, accumlatorInitialValue, 0x69, amountToAdd};
            size_t n = sizeof(program) / sizeof(program[0]);
            bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        }

        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.a, 2) == hex(accumlatorInitialValue, 2));
        
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.V) == 1) == expectedValue);
    }
}
#pragma endregion ADC - Add with Carry Tests

#pragma region AND - Compare Memory with Accumulator
TEST_CASE("AND Accumulator Correct", "[opcode][and]")
{
    auto [accumlatorInitialValue, amountToAdd, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, uint8_t>({
            {0x00, 0x00, 0x00},
            {0xFF, 0xFF, 0xFF},
            {0xFF, 0xFE, 0xFE},
            {0xAA, 0x55, 0x00}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(" << accumlatorInitialValue << ", " << amountToAdd << ", " << expectedValue << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        uint8_t program[] = {0xA9, accumlatorInitialValue, 0x29, amountToAdd};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.a, 2) == hex(accumlatorInitialValue, 2));
        
        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.a, 2) == hex(expectedValue, 2));
    }
}
#pragma endregion AND - Compare Memory with Accumulator

#pragma region ASL - Arithmetic Shift Left
TEST_CASE("ASL - Arithmetic Shift Left", "[opcode][asl]")
{
    auto [operation, valueToShift, expectedValue, expectedLocation] = 
        GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
            {0x0A, 0x6D, 0xDA, 0x0000},
            {0x0A, 0x6C, 0xD8, 0x0000},
            {0x06, 0x6D, 0xDA, 0x0001},
            {0x16, 0x6D, 0xDA, 0x0001},
            {0x0E, 0x6D, 0xDA, 0x0001},
            {0x1E, 0x6D, 0xDA, 0x0001},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(operation, 2) << ", 0x" << hex(valueToShift, 2) << ", 0x" << hex(expectedValue, 2) << ", 0x" << hex(expectedLocation, 2) << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        uint8_t program[] = {0xA9, valueToShift, operation, expectedLocation};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        if (operation == 0x0A)
        {
            REQUIRE(hex(bus.cpu.a, 2) == hex(expectedValue, 2));
        }
        else
        {
            REQUIRE(bus.read(expectedLocation, false) == expectedValue);
        }
    }
}

TEST_CASE("ASL Carry Set Correctly", "[opcode][asl]")
{
    auto [valueToShift, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x7F, false},
            {0x80, true},
            {0xFF, true},
            {0x00, false},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(valueToShift, 2) << ", " << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        uint8_t program[] = {0xA9, valueToShift, 0x0A};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.C) == 1) == expectedValue);
    }
}

TEST_CASE("ASL Negative Set Correctly", "[opcode][asl]")
{
    auto [valueToShift, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x3F, false},
            {0x40, true},
            {0x7F, true},
            {0x80, false},
            {0x00, false},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(valueToShift, 2) << ", " << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        uint8_t program[] = {0xA9, valueToShift, 0x0A};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.N) == 1) == expectedValue);
    }
}

TEST_CASE("ASL Zero Set Correctly", "[opcode][asl]")
{
    auto [valueToShift, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x7F, false},
            {0x80, true},
            {0x00, true},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(valueToShift, 2) << ", " << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        uint8_t program[] = {0xA9, valueToShift, 0x0A};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.Z) == 1) == expectedValue);
    }
}
#pragma endregion ASL - Arithmetic Shift Left

#pragma region BCC - Branch On Carry Clear
TEST_CASE("BCC Program Counter Correct", "[opcode][bcc]")
{
    auto [programCounterInitalValue, programOffset, expectedValue] = 
        GENERATE( table<uint16_t, uint8_t, uint16_t>({
            {0x00, 0x01, 0x03},
            {0x80, 0x80, 0x02},
            {0x00, 0x03, 0x05},
            {0x00, 0xFD, 0xFFFF},
            {0x7D, 0x80, 0xFFFF},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(programCounterInitalValue, 4) << ", 0x" << hex(programOffset, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0x90, programOffset};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(programCounterInitalValue, program, n, programCounterInitalValue);
        REQUIRE(bus.cpu.pc == programCounterInitalValue);
        bus.cpu.dumpRam(0x0000);

        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.pc, 4) == hex(expectedValue, 4));
    }
}
#pragma endregion BCC - Branch On Carry Clear

#pragma region BCS - Branch on Carry Set
TEST_CASE("BCS Program Counter Correct", "[opcode][bcs]")
{
    auto [programCounterInitalValue, programOffset, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, uint16_t>({
            {0x00, 0x01, 0x04},
            {0x80, 0x80, 0x03},
            {0x00, 0xFC, 0xFFFF},
            {0x7C, 0x80, 0xFFFF},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(programCounterInitalValue, 2) << ", 0x" << hex(programOffset, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0x38, 0xB0, programOffset};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(programCounterInitalValue, program, n, programCounterInitalValue);
        REQUIRE(bus.cpu.pc == programCounterInitalValue);

        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.pc, 4) == hex(expectedValue, 4));
    }
}
#pragma endregion BCS - Branch on Carry Set

#pragma region BEQ - Branch on Zero Set
TEST_CASE("BEQ Program Counter Correct", "[opcode][beq]")
{
    auto [programCounterInitalValue, programOffset, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, uint16_t>({
            {0x00, 0x01, 0x05},
            {0x80, 0x80, 0x04},
            {0x00, 0xFB, 0xFFFF},
            {0x7B, 0x80, 0xFFFF},
            {0x02, 0xFE, 0x04},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(programCounterInitalValue, 2) << ", 0x" << hex(programOffset, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, 0x00, 0xF0, programOffset};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(programCounterInitalValue, program, n, programCounterInitalValue);
        REQUIRE(bus.cpu.pc == programCounterInitalValue);

        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.pc, 4) == hex(expectedValue, 4));
    }
}
#pragma endregion BEQ - Branch on Zero Set

#pragma region BIT - Compare Memory with Accumulator
TEST_CASE("BIT Negative Set When Comparison Is Negative Number", "[opcode][bit]")
{
    auto [operation, accumulatorValue, valueToTest, expectedResult] = 
        GENERATE( table<uint8_t, uint8_t, uint8_t, bool>({
            {0x24, 0x7f, 0x7F, false}, // BIT Zero Page
            {0x24, 0x80, 0x7F, false}, // BIT Zero Page
            {0x24, 0x7F, 0x80, true}, // BIT Zero Page
            {0x24, 0x80, 0xFF, true}, // BIT Zero Page
            {0x24, 0xFF, 0x80, true}, // BIT Zero Page
            {0x2C, 0x7F, 0x7F, false}, // BIT Absolute
            {0x2C, 0x80, 0x7F, false}, // BIT Absolute
            {0x2C, 0x7F, 0x80, true}, // BIT Absolute
            {0x2C, 0x80, 0xFF, true}, // BIT Absolute
            {0x2C, 0xFF, 0x80, true}, // BIT Absolute
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(operation, 2) << ", 0x" << hex(accumulatorValue, 2) << ", 0x" << hex(valueToTest, 2) << ", " << expectedResult << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorValue, operation, 0x06, 0x00, 0x00, valueToTest};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x00, program, n, 0x00);
        REQUIRE(bus.cpu.pc == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.N) == 1) == expectedResult);
    }
}

TEST_CASE("BIT Overflow Set By Bit Six", "[opcode][bit]")
{
    auto [operation, accumulatorValue, valueToTest, expectedResult] = 
        GENERATE( table<uint8_t, uint8_t, uint8_t, bool>({
		    {0x24, 0x3F, 0x3F, false}, // BIT Zero Page
		    {0x24, 0x3F, 0x40, true}, // BIT Zero Page
		    {0x24, 0x40, 0x3F, false}, // BIT Zero Page
		    {0x24, 0x40, 0x7F, true}, // BIT Zero Page
		    {0x24, 0x7F, 0x40, true}, // BIT Zero Page
		    {0x24, 0x7F, 0x80, false}, // BIT Zero Page
		    {0x24, 0x80, 0x7F, true}, // BIT Zero Page
		    {0x24, 0xC0, 0xDF, true}, // BIT Zero Page
		    {0x24, 0xDF, 0xC0, true}, // BIT Zero Page
		    {0x24, 0x3F, 0x3F, false}, // BIT Zero Page
		    {0x24, 0xC0, 0xFF, true}, // BIT Zero Page
		    {0x24, 0xFF, 0xC0, true}, // BIT Zero Page
		    {0x24, 0x40, 0xFF, true}, // BIT Zero Page
		    {0x24, 0xFF, 0x40, true}, // BIT Zero Page
		    {0x24, 0xC0, 0x7F, true}, // BIT Zero Page
		    {0x24, 0x7F, 0xC0, true}, // BIT Zero Page
		    {0x2C, 0x3F, 0x3F, false}, // BIT Absolute
		    {0x2C, 0x3F, 0x40, true}, // BIT Absolute
		    {0x2C, 0x40, 0x3F, false}, // BIT Absolute
		    {0x2C, 0x40, 0x7F, true}, // BIT Absolute
		    {0x2C, 0x7F, 0x40, true}, // BIT Absolute
		    {0x2C, 0x7F, 0x80, false}, // BIT Absolute
		    {0x2C, 0x80, 0x7F, true}, // BIT Absolute
		    {0x2C, 0xC0, 0xDF, true}, // BIT Absolute
		    {0x2C, 0xDF, 0xC0, true}, // BIT Absolute
		    {0x2C, 0x3F, 0x3F, false}, // BIT Absolute
		    {0x2C, 0xC0, 0xFF, true}, // BIT Absolute
		    {0x2C, 0xFF, 0xC0, true}, // BIT Absolute
		    {0x2C, 0x40, 0xFF, true}, // BIT Absolute
		    {0x2C, 0xFF, 0x40, true}, // BIT Absolute
		    {0x2C, 0xC0, 0x7F, true}, // BIT Absolute
		    {0x2C, 0x7F, 0xC0, true}, // BIT Absolute
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(operation, 2) << ", 0x" << hex(accumulatorValue, 2) << ", 0x" << hex(valueToTest, 2) << ", " << expectedResult << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorValue, operation, 0x06, 0x00, 0x00, valueToTest};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x00, program, n, 0x00);
        REQUIRE(bus.cpu.pc == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.V) == 1) == expectedResult);
    }
}

TEST_CASE("BIT Zero Set When Comparison Is Zero", "[opcode][bit]")
{
    auto [operation, accumulatorValue, valueToTest, expectedResult] = 
        GENERATE( table<uint8_t, uint8_t, uint8_t, bool>({
		    {0x24, 0x00, 0x00, true}, // BIT Zero Page
		    {0x24, 0xFF, 0xFF, false}, // BIT Zero Page
		    {0x24, 0xAA, 0x55, true}, // BIT Zero Page
		    {0x24, 0x55, 0xAA, true}, // BIT Zero Page
		    {0x2C, 0x00, 0x00, true}, // BIT Absolute
		    {0x2C, 0xFF, 0xFF, false}, // BIT Absolute
		    {0x2C, 0xAA, 0x55, true}, // BIT Absolute
		    {0x2C, 0x55, 0xAA, true}, // BIT Absolute
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(operation, 2) << ", 0x" << hex(accumulatorValue, 2) << ", 0x" << hex(valueToTest, 2) << ", " << expectedResult << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorValue, operation, 0x06, 0x00, 0x00, valueToTest};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x00, program, n, 0x00);
        REQUIRE(bus.cpu.pc == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.Z) == 1) == expectedResult);
    }
}
#pragma endregion BIT - Compare Memory with Accumulator

#pragma region BMI - Branch if Negative Set
TEST_CASE("BMI Program Counter Correct", "[opcode][bmi]")
{
    auto [programCounterInitalValue, programOffset, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, uint16_t>({
            {0x00, 0x01, 0x05},
            {0x80, 0x80, 0x04},
            {0x00, 0xFB, 0xFFFF},
            {0x7B, 0x80, 0xFFFF},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(programCounterInitalValue, 2) << ", 0x" << hex(programOffset, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, 0x80, 0x30, programOffset};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(programCounterInitalValue, program, n, programCounterInitalValue);
        REQUIRE(bus.cpu.pc == programCounterInitalValue);

        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.pc, 4) == hex(expectedValue, 4));
    }
}
#pragma endregion BMI - Branch if Negative Set

#pragma region BNE - Branch On Result Not Zero
TEST_CASE("BNE Program Counter Correct", "[opcode][bne]")
{
    auto [programCounterInitalValue, programOffset, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, uint16_t>({
            {0x00, 0x01, 0x05},
            {0x80, 0x80, 0x04},
            {0x00, 0xFB, 0xFFFF},
            {0x7B, 0x80, 0xFFFF},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(programCounterInitalValue, 2) << ", 0x" << hex(programOffset, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, 0x01, 0xD0, programOffset};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(programCounterInitalValue, program, n, programCounterInitalValue);
        REQUIRE(bus.cpu.pc == programCounterInitalValue);

        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.pc, 4) == hex(expectedValue, 4));
    }
}
#pragma endregion BNE - Branch On Result Not Zero

#pragma region BPL - Branch if Negative Clear
TEST_CASE("BPL Program Counter Correct", "[opcode][bpl]")
{
    auto [programCounterInitalValue, programOffset, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, uint16_t>({
            {0x00, 0x01, 0x05},
            {0x80, 0x80, 0x04},
            {0x00, 0xFB, 0xFFFF},
            {0x7B, 0x80, 0xFFFF},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(programCounterInitalValue, 2) << ", 0x" << hex(programOffset, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, 0x79, 0x10, programOffset};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(programCounterInitalValue, program, n, programCounterInitalValue);
        REQUIRE(bus.cpu.pc == programCounterInitalValue);

        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.pc, 4) == hex(expectedValue, 4));
    }
}
#pragma endregion BPL - Branch if Negative Clear

#pragma region BRK - Simulate Interrupt Request (IRQ)
TEST_CASE("BRK Program Counter Set To Address At Break Vector Address", "[opcode][brk]")
{
    bus.cpu.reset();

    uint8_t program[] = {0x00};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);
    REQUIRE(bus.cpu.pc == 0x00);
    bus.write(0xFFFE, 0xBC);
    bus.write(0xFFFF, 0xCD);

    bus.cpu.tick();
    REQUIRE(bus.cpu.pc == 0xCDBC);
}

TEST_CASE("BRK Program Counter Stack Correct", "[opcode][brk]")
{
    bus.cpu.reset();

    uint8_t program[] = {0x00};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0xABCD, program, n, 0xABCD);

    uint8_t stackLocation = bus.cpu.stkp;

    bus.cpu.tick();

    REQUIRE(hex(bus.read(stackLocation + (0x0100 - 0)), 2) == hex(0xAB, 2));
    REQUIRE(hex(bus.read(stackLocation + (0x0100 - 1)), 2) == hex(0xCF, 2));
}

TEST_CASE("BRK Stack Pointer Correct", "[opcode][brk]")
{
    bus.cpu.reset();

    uint8_t program[] = {0x00};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0xABCD, program, n, 0xABCD);

    uint8_t stackLocation = bus.cpu.stkp;
    bus.cpu.tick();

    REQUIRE(hex(bus.cpu.stkp, 2) == hex(stackLocation - 3, 2));
}

TEST_CASE("BRK Stack Set Flag Operations Correctly", "[opcode][brk]")
{
    auto [operation, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t>({
            {0x38, 0x31}, //SEC Carry Flag Test
            {0xF8, 0x38}, //SED Decimal Flag Test
            {0x78, 0x34}, //SEI Interrupt Flag Test
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(operation, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0x58, operation, 0x00};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x00, program, n, 0x00);
        REQUIRE(bus.cpu.pc == 0x00);

        uint8_t stackLocation = bus.cpu.stkp;

        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(hex(bus.read(stackLocation + (0x0100 - 2)), 2) == hex(expectedValue, 2));
    }
}

TEST_CASE("BRK Stack Non Set Flag Operations Correctly", "[opcode][brk]")
{
    auto [accumulatorValue, memoryValue, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, uint8_t>({
            {0x01, 0x80, 0xB0}, //Negative
            {0x01, 0x7F, 0xF0}, //Overflow + Negative
            {0x00, 0x00, 0x32}, //Zero
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", 0x" << hex(memoryValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0x58, 0xA9, accumulatorValue, 0x69, memoryValue, 0x00};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x00, program, n, 0x00);
        REQUIRE(bus.cpu.pc == 0x00);

        uint8_t stackLocation = bus.cpu.stkp;

        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(hex(bus.read(stackLocation + (0x0100 - 2)), 2) == hex(expectedValue, 2));
    }
}
#pragma endregion BRK - Simulate Interrupt Request (IRQ)

#pragma region BVC - Branch if Overflow Clear
TEST_CASE("BVC Program Counter Correct", "[opcode][bvc]")
{
    auto [programCounterInitalValue, programOffset, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, uint16_t>({
            {0x00, 0x01, 0x03},
            {0x80, 0x80, 0x02},
            {0x00, 0xFD, 0xFFFF},
            {0x7D, 0x80, 0xFFFF},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(programCounterInitalValue, 2) << ", 0x" << hex(programOffset, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0x50, programOffset};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(programCounterInitalValue, program, n, programCounterInitalValue);
        REQUIRE(bus.cpu.pc == programCounterInitalValue);

        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.pc, 4) == hex(expectedValue, 4));
    }
}
#pragma endregion BVC - Branch if Overflow Clear

#pragma region BVS - Branch if Overflow Set
TEST_CASE("BVS Program Counter Correct", "[opcode][bvs]")
{
    auto [programCounterInitalValue, programOffset, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, uint16_t>({
            {0x00, 0x01, 0x07},
            {0x80, 0x80, 0x06},
            {0x00, 0xF9, 0xFFFF},
            {0x79, 0x80, 0xFFFF},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(programCounterInitalValue, 2) << ", 0x" << hex(programOffset, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, 0x01, 0x69, 0x7F, 0x70, programOffset};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(programCounterInitalValue, program, n, programCounterInitalValue);
        REQUIRE(bus.cpu.pc == programCounterInitalValue);

        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.pc, 4) == hex(expectedValue, 4));
    }
}
#pragma endregion BVS - Branch if Overflow Set

#pragma region CLC - Clear Carry Flag
TEST_CASE("CLC Carry Flag Cleared Correctly", "[opcode][clc]")
{
    bus.cpu.reset();

    uint8_t program[] = {0x18};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);

    bus.cpu.tick();
    REQUIRE(bus.cpu.GetFlag(bus.cpu.C) == false);
}
#pragma endregion CLC - Clear Carry Flag

#pragma region CLD - Clear Decimal Flag
TEST_CASE("CLD Carry Flag Set And Cleared Correctly", "[opcode][cld]")
{
    bus.cpu.reset();

    uint8_t program[] = {0xF9, 0xD8};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);

    bus.cpu.tick();
    bus.cpu.tick();
    REQUIRE(bus.cpu.GetFlag(bus.cpu.D) == false);
}
#pragma endregion CLD - Clear Decimal Flag

#pragma region CLI - Clear Interrupt Flag
TEST_CASE("CLI Interrupt Flag Cleared Correctly", "[opcode][cli]")
{
    bus.cpu.reset();

    uint8_t program[] = {0x58};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);

    bus.cpu.tick();
    REQUIRE(bus.cpu.GetFlag(bus.cpu.I) == false);
}
#pragma endregion CLI - Clear Interrupt Flag

#pragma region CLV - Clear Overflow Flag
TEST_CASE("CLV Overflow Flag Cleared Correctly", "[opcode][clv]")
{
    bus.cpu.reset();

    uint8_t program[] = {0xB8};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);

    bus.cpu.tick();
    REQUIRE(bus.cpu.GetFlag(bus.cpu.V) == false);
}
#pragma endregion CLV - Clear Overflow Flag

#pragma region CMP - Compare Memory With Accumulator
TEST_CASE("CMP Zero Flag Set When Values Match", "[opcode][cmp]")
{
    auto [accumulatorValue, memoryValue, expectedResult] = 
        GENERATE( table<uint8_t, uint8_t, bool>({
            {0x00, 0x00, true},
            {0xFF, 0x00, false},
            {0x00, 0xFF, false},
            {0xFF, 0xFF, true},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", 0x" << hex(memoryValue, 2) << ", " << expectedResult << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorValue, 0xC9, memoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x00, program, n, 0x00);
        REQUIRE(bus.cpu.pc == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.Z) == expectedResult);
    }
}

TEST_CASE("CMP Carry Flag Set When Accumulator Is Greater Than Or Equal", "[opcode][cmp]")
{
    auto [accumulatorValue, memoryValue, expectedResult] = 
        GENERATE( table<uint8_t, uint8_t, bool>({
            {0x00, 0x00, true},
            {0xFF, 0x00, true},
            {0x00, 0xFF, false},
            {0x00, 0x01, false},
            {0xFF, 0xFF, true},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", 0x" << hex(memoryValue, 2) << ", " << expectedResult << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorValue, 0xC9, memoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x00, program, n, 0x00);
        REQUIRE(bus.cpu.pc == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.C) == expectedResult);
    }
}

TEST_CASE("CMP Negative Flag Set When Result Is Negative", "[opcode][cmp]")
{
    auto [accumulatorValue, memoryValue, expectedResult] = 
        GENERATE( table<uint8_t, uint8_t, bool>({
            {0xFE, 0xFF, true},
            {0x81, 0x1, true},
            {0x81, 0x2, false},
            {0x79, 0x1, false},
            {0x00, 0x1, true},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", 0x" << hex(memoryValue, 2) << ", " << expectedResult << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorValue, 0xC9, memoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x00, program, n, 0x00);
        REQUIRE(bus.cpu.pc == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.N) == expectedResult);
    }
}
#pragma endregion CMP - Compare Memory With Accumulator

#pragma region CPX - Compare Memory With X Register
TEST_CASE("CPX Zero Flag Set When Values Match", "[opcode][cpx]")
{
    auto [accumulatorValue, memoryValue, expectedResult] = 
        GENERATE( table<uint8_t, uint8_t, bool>({
            {0x00, 0x00, true},
            {0xFF, 0x00, false},
            {0x00, 0xFF, false},
            {0xFF, 0xFF, true},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", 0x" << hex(memoryValue, 2) << ", " << expectedResult << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA2, accumulatorValue, 0xE0, memoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x00, program, n, 0x00);
        REQUIRE(bus.cpu.pc == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.Z) == expectedResult);
    }
}

TEST_CASE("CPX Carry Flag Set When Accumulator Is Greater Than Or Equal", "[opcode][cpx]")
{
    auto [accumulatorValue, memoryValue, expectedResult] = 
        GENERATE( table<uint8_t, uint8_t, bool>({
            {0x00, 0x00, true},
            {0xFF, 0x00, true},
            {0x00, 0xFF, false},
            {0x00, 0x01, false},
            {0xFF, 0xFF, true},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", 0x" << hex(memoryValue, 2) << ", " << expectedResult << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA2, accumulatorValue, 0xE0, memoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x00, program, n, 0x00);
        REQUIRE(bus.cpu.pc == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.C) == expectedResult);
    }
}

TEST_CASE("CPX Negative Flag Set When Result Is Negative", "[opcode][cpx]")
{
    auto [accumulatorValue, memoryValue, expectedResult] = 
        GENERATE( table<uint8_t, uint8_t, bool>({
            {0xFE, 0xFF, true},
            {0x81, 0x1, true},
            {0x81, 0x2, false},
            {0x79, 0x1, false},
            {0x00, 0x1, true},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", 0x" << hex(memoryValue, 2) << ", " << expectedResult << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA2, accumulatorValue, 0xE0, memoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x00, program, n, 0x00);
        REQUIRE(bus.cpu.pc == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.N) == expectedResult);
    }
}
#pragma endregion CPX - Compare Memory With X Register

#pragma region CPY - Compare Memory With X Register
TEST_CASE("CPY Zero Flag Set When Values Match", "[opcode][cpy]")
{
    auto [accumulatorValue, memoryValue, expectedResult] = 
        GENERATE( table<uint8_t, uint8_t, bool>({
            {0x00, 0x00, true},
            {0xFF, 0x00, false},
            {0x00, 0xFF, false},
            {0xFF, 0xFF, true},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", 0x" << hex(memoryValue, 2) << ", " << expectedResult << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA0, accumulatorValue, 0xC0, memoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x00, program, n, 0x00);
        REQUIRE(bus.cpu.pc == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.Z) == expectedResult);
    }
}

TEST_CASE("CPY Carry Flag Set When Accumulator Is Greater Than Or Equal", "[opcode][cpy]")
{
    auto [accumulatorValue, memoryValue, expectedResult] = 
        GENERATE( table<uint8_t, uint8_t, bool>({
            {0x00, 0x00, true},
            {0xFF, 0x00, true},
            {0x00, 0xFF, false},
            {0x00, 0x01, false},
            {0xFF, 0xFF, true},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", 0x" << hex(memoryValue, 2) << ", " << expectedResult << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA0, accumulatorValue, 0xC0, memoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x00, program, n, 0x00);
        REQUIRE(bus.cpu.pc == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.C) == expectedResult);
    }
}

TEST_CASE("CPY Negative Flag Set When Result Is Negative", "[opcode][cpy]")
{
    auto [accumulatorValue, memoryValue, expectedResult] = 
        GENERATE( table<uint8_t, uint8_t, bool>({
            {0xFE, 0xFF, true},
            {0x81, 0x1, true},
            {0x81, 0x2, false},
            {0x79, 0x1, false},
            {0x00, 0x1, true},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", 0x" << hex(memoryValue, 2) << ", " << expectedResult << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA0, accumulatorValue, 0xC0, memoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x00, program, n, 0x00);
        REQUIRE(bus.cpu.pc == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.N) == expectedResult);
    }
}
#pragma endregion CPY - Compare Memory With X Register

#pragma region DEC - Decrement Memory by One
TEST_CASE("DEC Memory Has Correct Value", "[opcode][dec]")
{
    auto [initialMemoryValue, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t>({
            {0x00, 0xFF},
            {0xFF, 0xFE},
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(initialMemoryValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xC6, 0x03, 0x00, initialMemoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        
        bus.cpu.tick();
        REQUIRE(bus.read(0x0003, false) == expectedValue);
    }
}

TEST_CASE("DEC Zero Set Correctly", "[opcode][dec]")
{
    auto [initialMemoryValue, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x00, false},
            {0x01, true},
            {0x02, false},
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(initialMemoryValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xC6, 0x03, 0x00, initialMemoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.Z) == 1) == expectedValue);
    }
}

TEST_CASE("DEC Negative Set Correctly", "[opcode][dec]")
{
    auto [initialMemoryValue, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x80, false},
            {0x81, true},
            {0x00, true}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(initialMemoryValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xC6, 0x03, 0x00, initialMemoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.N) == 1) == expectedValue);
    }
}
#pragma endregion DEC - Decrement Memory by One

#pragma region DEX - Decrement X by One
TEST_CASE("DEX Memory Has Correct Value", "[opcode][dex]")
{
    auto [initialMemoryValue, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t>({
            {0x00, 0xFF},
            {0xFF, 0xFE},
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(initialMemoryValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.x == 0x00);

        uint8_t program[] = {0xA2, initialMemoryValue, 0xCA};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        
        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE(bus.cpu.x == expectedValue);
    }
}

TEST_CASE("DEX Zero Set Correctly", "[opcode][dex]")
{
    auto [initialMemoryValue, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x00, false},
            {0x01, true},
            {0x02, false},
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(initialMemoryValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA2, initialMemoryValue, 0xCA};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        
        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.Z) == 1) == expectedValue);
    }
}

TEST_CASE("DEX Negative Set Correctly", "[opcode][dex]")
{
    auto [initialMemoryValue, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x80, false},
            {0x81, true},
            {0x00, true}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(initialMemoryValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA2, initialMemoryValue, 0xCA};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.N) == 1) == expectedValue);
    }
}
#pragma endregion DEX - Decrement X by One

#pragma region DEY - Decrement Y by One
TEST_CASE("DEY Memory Has Correct Value", "[opcode][dey]")
{
    auto [initialMemoryValue, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t>({
            {0x00, 0xFF},
            {0xFF, 0xFE},
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(initialMemoryValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.y == 0x00);

        uint8_t program[] = {0xA0, initialMemoryValue, 0x88};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        
        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.y, 2) == hex(expectedValue, 2));
    }
}

TEST_CASE("DEY Zero Set Correctly", "[opcode][dey]")
{
    auto [initialMemoryValue, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x00, false},
            {0x01, true},
            {0x02, false},
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(initialMemoryValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA0, initialMemoryValue, 0x88};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        
        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.Z) == 1) == expectedValue);
    }
}

TEST_CASE("DEY Negative Set Correctly", "[opcode][dey]")
{
    auto [initialMemoryValue, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x80, false},
            {0x81, true},
            {0x00, true}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(initialMemoryValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA0, initialMemoryValue, 0x88};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.N) == 1) == expectedValue);
    }
}
#pragma endregion DEY - Decrement Y by One

#pragma region EOR - Exclusive OR Compare Accumulator With Memory
TEST_CASE("EOR Accumulator Correct", "[opcode][eor]")
{
    auto [accumulatorValue, memoryValue, expectedResult] = 
        GENERATE( table<uint8_t, uint8_t, uint8_t>({
            {0x00, 0x00, 0x00},
            {0xFF, 0x00, 0xFF},
            {0x00, 0xFF, 0xFF},
            {0x55, 0xAA, 0xFF},
            {0xFF, 0xFF, 0x00},
        })
    );

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", 0x" << hex(memoryValue, 2) << ", 0x" << hex(expectedResult, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorValue, 0x49, memoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        
        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE(bus.cpu.a == expectedResult);
    }
}

TEST_CASE("EOR Negative Flag Correct", "[opcode][eor]")
{
    auto [accumulatorValue, memoryValue, expectedResult] = 
        GENERATE( table<uint8_t, uint8_t, bool>({
            {0xFF, 0xFF, false},
            {0x80, 0x7F, true},
            {0x40, 0x3F, false},
            {0xFF, 0x7F, true},
        })
    );

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", 0x" << hex(memoryValue, 2) << ", 0x" << hex(expectedResult, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorValue, 0x49, memoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        
        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.N) == 1) == expectedResult);
    }
}

TEST_CASE("EOR Zero Flag Correct", "[opcode][eor]")
{
    auto [accumulatorValue, memoryValue, expectedResult] = 
        GENERATE( table<uint8_t, uint8_t, bool>({
            {0xFF, 0xFF, true},
            {0x80, 0x7F, false},
        })
    );
    
    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", 0x" << hex(memoryValue, 2) << ", 0x" << hex(expectedResult, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorValue, 0x49, memoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        
        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.Z) == 1) == expectedResult);
    }
}
#pragma endregion EOR - Exclusive OR Compare Accumulator With Memory

#pragma region INC - Increment Memory by One
TEST_CASE("INC Memory Has Correct Value", "[opcode][inc]")
{
    auto [initialMemoryValue, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x00, 0x01},
            {0xFF, 0x00},
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(initialMemoryValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xE6, 0x03, 0x00, initialMemoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        
        bus.cpu.tick();
        REQUIRE(bus.read(0x0003, false) == expectedValue);
    }
}

TEST_CASE("INC Zero Set Correctly", "[opcode][inc]")
{
    auto [initialMemoryValue, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x00, false},
            {0xFF, true},
            {0xFE, false},
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(initialMemoryValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xE6, 0x03, 0x00, initialMemoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.Z) == 1) == expectedValue);
    }
}

TEST_CASE("INC Negative Set Correctly", "[opcode][inc]")
{
    auto [initialMemoryValue, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x78, false},
            {0x80, true},
            {0x00, false}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(initialMemoryValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xE6, 0x03, 0x00, initialMemoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.N) == 1) == expectedValue);
    }
}
#pragma endregion INC - Increment Memory by One

#pragma region INX - Increment X by One
TEST_CASE("INX Memory Has Correct Value", "[opcode][inx]")
{
    auto [initialMemoryValue, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x00, 0x01},
            {0xFF, 0x00},
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(initialMemoryValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA2, initialMemoryValue, 0xE8};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        
        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE(bus.cpu.x == expectedValue);
    }
}

TEST_CASE("INX Zero Set Correctly", "[opcode][inx]")
{
    auto [initialMemoryValue, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x00, false},
            {0xFF, true},
            {0xFE, false},
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(initialMemoryValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA2, initialMemoryValue, 0xE8};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        
        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.Z) == 1) == expectedValue);
    }
}

TEST_CASE("INX Negative Set Correctly", "[opcode][inx]")
{
    auto [initialMemoryValue, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x78, false},
            {0x80, true},
            {0x00, false}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(initialMemoryValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA2, initialMemoryValue, 0xE8};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.N) == 1) == expectedValue);
    }
}
#pragma endregion INX - Increment X by One

#pragma region INY - Increment Y by One
TEST_CASE("INY Memory Has Correct Value", "[opcode][iny]")
{
    auto [initialMemoryValue, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x00, 0x01},
            {0xFF, 0x00},
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(initialMemoryValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA0, initialMemoryValue, 0xC8};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        
        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE(bus.cpu.y == expectedValue);
    }
}

TEST_CASE("INY Zero Set Correctly", "[opcode][iny]")
{
    auto [initialMemoryValue, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x00, false},
            {0xFF, true},
            {0xFE, false},
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(initialMemoryValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA0, initialMemoryValue, 0xC8};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        
        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.Z) == 1) == expectedValue);
    }
}

TEST_CASE("INY Negative Set Correctly", "[opcode][iny]")
{
    auto [initialMemoryValue, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x78, false},
            {0x80, true},
            {0x00, false}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(initialMemoryValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA0, initialMemoryValue, 0xC8};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.N) == 1) == expectedValue);
    }
}
#pragma endregion INY - Increment Y by One

#pragma region JMP - Jump to New Location
TEST_CASE("JMP Program Counter Set Correctly After Jump", "[opcode][jmp]")
{
    bus.cpu.reset();

    uint8_t program[] = {0x4C, 0x08, 0x00};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);

    bus.cpu.tick();
    REQUIRE(hex(bus.cpu.pc, 4) == hex(0x08, 4));
}

TEST_CASE("JMP Program Counter Set Correctly After Indirect Jump", "[opcode][jmp]")
{
    bus.cpu.reset();

    uint8_t program[] = {0x6C, 0x03, 0x00, 0x08, 0x00};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);

    bus.cpu.tick();
    REQUIRE(hex(bus.cpu.pc, 4) == hex(0x08, 4));
}

TEST_CASE("JMP Indirect Wraps Correct If MSB IS FF", "[opcode][jmp]")
{
    bus.cpu.reset();

    uint8_t program[] = {0x6C, 0xFF, 0x01, 0x08, 0x00};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);
    bus.write(0x01FE, 0x6C);
    bus.write(0x01FF, 0x03);
    bus.write(0x0100, 0x02);

    bus.cpu.tick();
    REQUIRE(hex(bus.cpu.pc, 4) == hex(0x0203, 4));
}
#pragma endregion JMP - Jump to New Location

#pragma region JSR - Jump to SubRoutine
TEST_CASE("JSR Stack Loads Correct Value", "[opcode][jsr]")
{
    bus.cpu.reset();

    uint8_t program[] = {0x20, 0xCC, 0xCC};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0xBBAA, program, n, 0xBBAA);

    uint8_t stackLocation = bus.cpu.stkp;
    bus.cpu.tick();

    REQUIRE(hex(bus.read(stackLocation + 0x0100), 4) == hex(0xBB, 4));
    REQUIRE(hex(bus.read(stackLocation + 0x0100 - 1), 4) == hex(0xAC, 4));
}

TEST_CASE("JSR Program Counter Correct", "[opcode][jsr]")
{
    bus.cpu.reset();

    uint8_t program[] = {0x20, 0xCC, 0xCC};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);
    bus.cpu.tick();

    REQUIRE(hex(bus.cpu.pc, 4) == hex(0xCCCC, 4));
}

TEST_CASE("JSR Stack Pointer Correct", "[opcode][jsr]")
{
    bus.cpu.reset();

    uint8_t program[] = {0x20, 0xCC, 0xCC};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0xBBAA, program, n, 0xBBAA);

    uint8_t stackLocation = bus.cpu.stkp;
    bus.cpu.tick();

    REQUIRE(hex(bus.cpu.stkp, 4) == hex(stackLocation - 2, 4));
}
#pragma endregion JSR - Jump to SubRoutine

#pragma region LDA - Load Accumulator with Memory
TEST_CASE("LDA Accumulator Has Correct Value", "[opcode][lda]")
{
    bus.cpu.reset();

    REQUIRE(bus.cpu.a == 0x00);

    uint8_t program[] = {0xA9, 0x03};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);

    bus.cpu.tick();
    REQUIRE(bus.cpu.a == 0x03);
}

TEST_CASE("LDA Zero Set Correctly", "[opcode][lda]")
{
    auto [valueToLoad, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x00, true},
            {0x03, false},
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(valueToLoad, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        uint8_t program[] = {0xA9, valueToLoad};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.Z) == 1) == expectedValue);
    }
}

TEST_CASE("LDA Negative Set Correctly", "[opcode][lda]")
{
    auto [valueToLoad, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x00, false},
            {0x79, false},
            {0x80, true},
            {0xFF, true}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(valueToLoad, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        uint8_t program[] = {0xA9, valueToLoad};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.N) == 1) == expectedValue);
    }
}
#pragma endregion LDA - Load Accumulator with Memory

#pragma region LDX - Load X with Memory
TEST_CASE("LDX X-Register Has Correct Value", "[opcode][ldx]")
{
    bus.cpu.reset();

    REQUIRE(bus.cpu.x == 0x00);

    uint8_t program[] = {0xA2, 0x03};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);

    bus.cpu.tick();
    REQUIRE(bus.cpu.x == 0x03);
}

TEST_CASE("LDX Zero Set Correctly", "[opcode][ldx]")
{
    auto [valueToLoad, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x00, true},
            {0x03, false},
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(valueToLoad, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.x == 0x00);

        uint8_t program[] = {0xA9, valueToLoad};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.Z) == 1) == expectedValue);
    }
}

TEST_CASE("LDX Negative Set Correctly", "[opcode][ldx]")
{
    auto [valueToLoad, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x00, false},
            {0x79, false},
            {0x80, true},
            {0xFF, true}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(valueToLoad, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.x == 0x00);

        uint8_t program[] = {0xA9, valueToLoad};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.N) == 1) == expectedValue);
    }
}
#pragma endregion LDX - Load X with Memory

#pragma region LDY - Load Y with Memory
TEST_CASE("LDY Y-Register Has Correct Value", "[opcode][ldy]")
{
    bus.cpu.reset();

    REQUIRE(bus.cpu.y == 0x00);

    uint8_t program[] = {0xA0, 0x03};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);

    bus.cpu.tick();
    REQUIRE(bus.cpu.y == 0x03);
}

TEST_CASE("LDY Zero Set Correctly", "[opcode][ldy]")
{
    auto [valueToLoad, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x00, true},
            {0x03, false},
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(valueToLoad, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.y == 0x00);

        uint8_t program[] = {0xA0, valueToLoad};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.Z) == 1) == expectedValue);
    }
}

TEST_CASE("LDY Negative Set Correctly", "[opcode][ldy]")
{
    auto [valueToLoad, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x00, false},
            {0x79, false},
            {0x80, true},
            {0xFF, true}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(valueToLoad, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.y == 0x00);

        uint8_t program[] = {0xA0, valueToLoad};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.N) == 1) == expectedValue);
    }
}
#pragma endregion LDY - Load Y with Memory

#pragma region LSR - Logical Shift Right
TEST_CASE("LSR Negative Set Correctly", "[opcode][lsr]")
{
    auto [accumulatorValue, carryBitSet, expectedValue] = 
        GENERATE( table<uint8_t, bool, bool>({
            {0xFF, false, false},
            {0xFE, false, false},
            {0xFF, true, false},
            {0x00, true, false},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", 0x" << hex(carryBitSet, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t carryOperation = carryBitSet ? 0x38 : 0x18;

        uint8_t program[] = {carryOperation, 0xA9, accumulatorValue, 0x4A};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.N) == expectedValue);
    }
}

TEST_CASE("LSR Zero Set Correctly", "[opcode][lsr]")
{
    auto [accumulatorValue, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x01, true},
            {0x02, false}
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorValue, 0x4A};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.Z) == expectedValue);
    }
}

TEST_CASE("LSR Carry Set Correctly", "[opcode][lsr]")
{
    auto [accumulatorValue, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x01, true},
            {0x02, false}
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", " << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorValue, 0x4A};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.C) == expectedValue);
    }
}

TEST_CASE("LSR Correct Value Stored", "[opcode][lsr]")
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

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(operation, 2) << ", 0x" << hex(valueToShift, 2) << ", 0x" << hex(expectedValue, 2) << ", 0x" << hex(expectedLocation, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, valueToShift, operation, expectedLocation};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        uint8_t actualValue;
        if (operation == 0x4A)
        {
            actualValue = bus.cpu.a;
        }
        else
        {
            actualValue = bus.read(expectedLocation);
        }
        REQUIRE(actualValue == expectedValue);
    }
}
#pragma endregion LSR - Logical Shift Right

#pragma region ORA - Bitwise OR Compare Memory with Accumulator
TEST_CASE("ORA Accumulator Correct", "[opcode][ora]")
{
    auto [accumulatorValue, memoryValue, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, uint8_t>({
            {0x00, 0x00, 0x00},
            {0xFF, 0xFF, 0xFF},
            {0x55, 0xAA, 0xFF},
            {0xAA, 0x55, 0xFF},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", 0x" << hex(memoryValue, 2) << ", " << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorValue, 0x09, memoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(hex(bus.cpu.a, 2) == hex(expectedValue, 2));
    }
}

TEST_CASE("ORA Zero Set Correctly", "[opcode][ora]")
{
    auto [accumulatorValue, memoryValue, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, bool>({
            {0x00, 0x00, true},
            {0xFF, 0xFF, false},
            {0x00, 0x01, false}
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", 0x" << hex(memoryValue, 2) << ", " << expectedValue << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorValue, 0x09, memoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.Z) == expectedValue);
    }
}

TEST_CASE("ORA Negative Set Correctly", "[opcode][ora]")
{
    auto [accumulatorValue, memoryValue, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, bool>({
            {0x7F, 0x80, true},
            {0x79, 0x00, false},
            {0xFF, 0xFF, true},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", 0x" << hex(memoryValue, 2) << ", " << expectedValue << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorValue, 0x09, memoryValue};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.N) == expectedValue);
    }
}
#pragma endregion ORA - Bitwise OR Compare Memory with Accumulator

#pragma region PHA - Push Accumulator Onto Stack
TEST_CASE("PHA Stack Has Correct Value", "[opcode][pha]")
{
    uint8_t program[] = {0xA9, 0x03, 0x48};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);

    uint8_t stackLocation = bus.cpu.stkp;

    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE(hex(bus.read(stackLocation + 0x0100), 2) == hex(0x03, 2));
}

TEST_CASE("PHA Stack Pointer Has Correct Value", "[opcode][pha]")
{
    uint8_t program[] = {0xA9, 0x03, 0x48};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);

    uint8_t stackLocation = bus.cpu.stkp;

    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE(hex(bus.cpu.stkp, 2) == hex(stackLocation - 1, 2));
}

TEST_CASE("PHA Stack Pointer Has Correct Value When Wrapping", "[opcode][pha]")
{
    uint8_t program[] = {0x9A, 0x48};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);

    uint8_t stackLocation = bus.cpu.stkp;

    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE(hex(bus.cpu.stkp, 2) == hex(0xFF, 2));
}
#pragma endregion PHA - Push Accumulator Onto Stack

#pragma region PHP - Push Flags Onto Stack
TEST_CASE("PHP Stack Set Flag Operations Correctly", "[opcode][php]")
{
    auto [operation, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t>({
            {0x38, 0x31},
            {0xF8, 0x38},
            {0x78, 0x34},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(operation, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0x58, operation, 0x08};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        uint8_t stackLocation = bus.cpu.stkp;

        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(hex(bus.read(stackLocation + 0x0100), 2) == hex(expectedValue, 2));
    }
}

TEST_CASE("PHP Stack Non Set Flag Operations Correctly", "[opcode][php]")
{
    auto [accumulatorValue, memoryValue, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, uint8_t>({
            {0x01, 0x80, 0xB0}, //Negative
            {0x01, 0x7F, 0xF0}, //Overflow + Negative
            {0x00, 0x00, 0x32}, //Zero
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", 0x" << hex(memoryValue, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0x58, 0xA9, accumulatorValue, 0x69, memoryValue, 0x08};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        uint8_t stackLocation = bus.cpu.stkp;
        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(hex(bus.read(stackLocation + 0x0100), 2) == hex(expectedValue, 2));
    }
}

TEST_CASE("PHP Stack Pointer Has Correct Value", "[opcode][php]")
{
    bus.cpu.reset();

    uint8_t program[] = {0x08};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);

    uint8_t stackLocation = bus.cpu.stkp;

    bus.cpu.tick();

    REQUIRE(hex(bus.cpu.stkp, 2) == hex(stackLocation - 1, 2));
}
#pragma endregion PHP - Push Flags Onto Stack 

#pragma region PLA - Pull From Stack to Accumulator
TEST_CASE("PLA Accumulator Has Correct Value", "[opcode][pla]")
{
    bus.cpu.reset();

    uint8_t program[] = {0xA9, 0x03, 0x48, 0xA9, 0x00, 0x68};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);

    bus.cpu.tick();

    REQUIRE(hex(bus.cpu.a, 2) == hex(0x03, 2));
}

TEST_CASE("PLA Zero Flag Has Correct Value", "[opcode][pla]")
{
    auto [valueToLoad, expectedResult] = 
        GENERATE( table<uint8_t, bool>({
            {0x00, true},
            {0x01, false},
            {0xFF, false},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(valueToLoad, 2) << ", " << expectedResult << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, valueToLoad, 0x48, 0x68};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        uint8_t stackLocation = bus.cpu.stkp;
        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE((bus.cpu.GetFlag(bus.cpu.Z) == 1) == expectedResult);
    }
}

TEST_CASE("PLA Negative Flag Has Correct Value", "[opcode][pla]")
{
    auto [valueToLoad, expectedResult] = 
        GENERATE( table<uint8_t, bool>({
            {0x7F, false},
            {0x80, true},
            {0xFF, true},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(valueToLoad, 2) << ", " << expectedResult << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, valueToLoad, 0x48, 0x68};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        uint8_t stackLocation = bus.cpu.stkp;
        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE((bus.cpu.GetFlag(bus.cpu.N) == 1) == expectedResult);
    }
}
#pragma endregion PLA - Pull From Stack to Accumulator

#pragma region PLP - Pull From Stack to Flags
TEST_CASE("PLP Carry Flag Set Correctly", "[opcode][plp]")
{
    bus.cpu.reset();

    uint8_t program[] = {0xA9, 0x01, 0x48, 0x28};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);

    bus.cpu.tick();
    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE((bus.cpu.GetFlag(bus.cpu.C) == 1) == true);
}

TEST_CASE("PLP Zero Flag Set Correctly", "[opcode][plp]")
{
    bus.cpu.reset();

    uint8_t program[] = {0xA9, 0x02, 0x48, 0x28};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);

    bus.cpu.tick();
    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE((bus.cpu.GetFlag(bus.cpu.Z) == 1) == true);
}

TEST_CASE("PLP Decimal Flag Set Correctly", "[opcode][plp]")
{
    bus.cpu.reset();

    uint8_t program[] = {0xA9, 0x08, 0x48, 0x28};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);

    bus.cpu.tick();
    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE((bus.cpu.GetFlag(bus.cpu.D) == 1) == true);
}

TEST_CASE("PLP Interrupt Flag Set Correctly", "[opcode][plp]")
{
    bus.cpu.reset();

    uint8_t program[] = {0xA9, 0x04, 0x48, 0x28};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);

    bus.cpu.tick();
    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE((bus.cpu.GetFlag(bus.cpu.I) == 1) == true);
}

TEST_CASE("PLP Overflow Flag Set Correctly", "[opcode][plp]")
{
    bus.cpu.reset();

    uint8_t program[] = {0xA9, 0x40, 0x48, 0x28};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);

    bus.cpu.tick();
    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE((bus.cpu.GetFlag(bus.cpu.V) == 1) == true);
}

TEST_CASE("PLP Negative Flag Set Correctly", "[opcode][plp]")
{
    bus.cpu.reset();

    uint8_t program[] = {0xA9, 0x80, 0x48, 0x28};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);

    bus.cpu.tick();
    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE((bus.cpu.GetFlag(bus.cpu.N) == 1) == true);
}
#pragma endregion PLP - Pull From Stack to Flags

#pragma region ROL - Rotate Left
TEST_CASE("ROL Negative Set Correctly", "[opcode][rol]")
{
    auto [accumulatorValue, expectedResult] = 
        GENERATE( table<uint8_t, bool>({
            {0x40, true},
            {0x3F, false},
            {0x80, false},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", " << hex(expectedResult, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorValue, 0x2A};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.N) == expectedResult);
    }
}

TEST_CASE("ROL Zero Set Correctly", "[opcode][rol]")
{
    auto [carryFlagSet, expectedResult] = 
        GENERATE( table<bool, bool>({
            {true, false},
            {false, true},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(" << carryFlagSet << ", " << expectedResult << ") works")
    {
        bus.cpu.reset();

        uint8_t carryOperation = carryFlagSet ? 0x38 : 0x18;

        uint8_t program[] = {carryOperation, 0x2A};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE((bus.cpu.GetFlag(bus.cpu.Z) == 1) == expectedResult);
    }
}

TEST_CASE("ROL Carry Flag Set Correctly", "[opcode][rol]")
{
    auto [accumulatorValue, expectedResult] = 
        GENERATE( table<uint8_t, bool>({
            {0x80, true},
            {0x7F, false},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", " << expectedResult << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorValue, 0x2A};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.C) == expectedResult);
    }
}

TEST_CASE("ROL Correct Value Stored", "[opcode][rol]")
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

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(operation, 2) << ", 0x" << hex(valueToRotate, 2) << ", " << hex(expectedValue, 2) << ", " << hex(expectedLocation, 2) << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.pc == 0x00);

        uint8_t program[] = {0xA9, valueToRotate, operation, expectedLocation};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        uint8_t actualResult;
        if (operation == 0x2A)
        {
            actualResult = bus.cpu.a;
        }
        else
        {
            actualResult = bus.read(expectedLocation);
        }

        REQUIRE(hex(actualResult, 2) == hex(expectedValue, 2));
    }
}
#pragma endregion ROL - Rotate Left

#pragma region ROR - Rotate Right
TEST_CASE("ROR Negative Set Correctly", "[opcode][ror]")
{
    auto [accumulatorValue, carryBitSet, expectedValue] = 
        GENERATE( table<uint8_t, bool, bool>({
            {0xFF, false, false},
            {0xFE, false, false},
            {0xFF, true, true},
            {0x00, true, true},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", " << carryBitSet << ", " << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t carryOperation = carryBitSet ? 0x38 : 0x18;

        uint8_t program[] = {carryOperation, 0xA9, accumulatorValue, 0x6A};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.N) == expectedValue);
    }
}

TEST_CASE("ROR Zero Set Correctly", "[opcode][ror]")
{
    auto [accumulatorValue, carryBitSet, expectedResult] = 
        GENERATE( table<uint8_t, bool, bool>({
		    {0x00, false, true},
		    {0x00, true, false},
		    {0x01, false, true},
		    {0x01, true, false},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(" << hex(accumulatorValue, 2) << ", " << carryBitSet << ", " << expectedResult << ") works")
    {
        bus.cpu.reset();

        uint8_t carryOperation = carryBitSet ? 0x38 : 0x18;

        uint8_t program[] = {carryOperation, 0xA9, accumulatorValue, 0x6A};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.Z) == expectedResult);
    }
}

TEST_CASE("ROR Carry Flag Set Correctly", "[opcode][ror]")
{
    auto [accumulatorValue, expectedResult] = 
        GENERATE( table<uint8_t, bool>({
            {0x01, true},
            {0x02, false},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumulatorValue, 2) << ", " << expectedResult << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorValue, 0x6A};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.C) == expectedResult);
    }
}

TEST_CASE("ROR Correct Value Stored", "[opcode][ror]")
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

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(operation, 2) << ", 0x" << hex(valueToRotate, 2) << ", " << hex(expectedValue, 2) << ", " << hex(expectedLocation, 2) << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.pc == 0x00);

        uint8_t program[] = {0xA9, valueToRotate, operation, expectedLocation};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        uint8_t actualResult;
        if (operation == 0x6A)
        {
            actualResult = bus.cpu.a;
        }
        else
        {
            actualResult = bus.read(expectedLocation);
        }

        REQUIRE(hex(actualResult, 2) == hex(expectedValue, 2));
    }
}
#pragma endregion ROR - Rotate Right

#pragma region RTI - Return from Interrupt
TEST_CASE("RTI Program Counter Correct", "[opcode][rti]")
{
    bus.cpu.reset();

    uint8_t program[] = {0x00};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0xABCD, program, n, 0xABCD);

    //The Reset Vector Points to 0x0000 by default, so load the RTI instruction there.
    bus.write(0x00, 0x40);

    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE(hex(bus.cpu.pc, 4) == hex(0xABCF, 4));
}

TEST_CASE("RTI Carry Flag Set Correctly", "[opcode][rti]")
{
    bus.cpu.reset();

    //Load Accumulator and Transfer to Stack, Clear Accumulator, and Return from Interrupt
    uint8_t program[] = {0xA9, 0x01, 0x48, 0x40};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);

    bus.cpu.tick();
    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE((bus.cpu.GetFlag(bus.cpu.C) == 1) == true);
}

TEST_CASE("RTI Zero Flag Set Correctly", "[opcode][rti]")
{
    bus.cpu.reset();

    //Load Accumulator and Transfer to Stack, Clear Accumulator, and Return from Interrupt
    uint8_t program[] = {0xA9, 0x02, 0x48, 0x40};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);

    bus.cpu.tick();
    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE((bus.cpu.GetFlag(bus.cpu.Z) == 1) == true);
}

TEST_CASE("RTI Interrupt Flag Set Correctly", "[opcode][rti]")
{
    bus.cpu.reset();

    //Load Accumulator and Transfer to Stack, Clear Accumulator, and Return from Interrupt
    uint8_t program[] = {0xA9, 0x04, 0x48, 0x40};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);

    bus.cpu.tick();
    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE((bus.cpu.GetFlag(bus.cpu.I) == 1) == true);
}

TEST_CASE("RTI Decimal Flag Set Correctly", "[opcode][rti]")
{
    bus.cpu.reset();

    //Load Accumulator and Transfer to Stack, Clear Accumulator, and Return from Interrupt
    uint8_t program[] = {0xA9, 0x08, 0x48, 0x40};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);

    bus.cpu.tick();
    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE((bus.cpu.GetFlag(bus.cpu.D) == 1) == true);
}

TEST_CASE("RTI Overflow Flag Set Correctly", "[opcode][rti]")
{
    bus.cpu.reset();

    uint8_t program[] = {0xA9, 0x40, 0x48, 0x28};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);

    bus.cpu.tick();
    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE((bus.cpu.GetFlag(bus.cpu.V) == 1) == true);
}

TEST_CASE("RTI Negative Flag Set Correctly", "[opcode][rti]")
{
    bus.cpu.reset();

    uint8_t program[] = {0xA9, 0x80, 0x48, 0x28};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);

    bus.cpu.tick();
    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE((bus.cpu.GetFlag(bus.cpu.N) == 1) == true);
}
#pragma endregion RTI - Return from Interrupt

#pragma region RTS - Return from SubRoutine
TEST_CASE("RTS Program Counter Has Correct Value", "[opcode][rts]")
{
    bus.cpu.reset();

    uint8_t program[] = {0x20, 0x04, 0x00, 0x00, 0x60};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);

    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE(hex(bus.cpu.pc, 4) == hex(0x03, 4));
}

TEST_CASE("RTS Stack Pointer Has Correct Value", "[opcode][rts]")
{
    bus.cpu.reset();

    uint8_t program[] = {0x60};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0xBBAA, program, n, 0xBBAA);

    uint8_t stackLocation = bus.cpu.stkp;

    bus.cpu.tick();

    REQUIRE(hex(bus.cpu.stkp, 4) == hex(stackLocation + 2, 4));
}
#pragma endregion RTS - Return from SubRoutine

#pragma region SBC - Subtraction With Borrow
TEST_CASE("SBC Accumulator Correct When Not In BDC Mode", "[opcode][sbc]")
{
    auto [accumlatorInitialValue, amountToSubtract, CarryFlagSet, expectedValue] = 
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
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumlatorInitialValue, 2) << ", 0x" << hex(amountToSubtract, 2) << ", " << CarryFlagSet << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        if (CarryFlagSet)
        { 
            uint8_t program[] = {0x38, 0xA9, accumlatorInitialValue, 0xE9, amountToSubtract};
            size_t n = sizeof(program) / sizeof(program[0]);
            bus.cpu.LoadProgram(0x0000, program, n, 0x00);
            bus.cpu.tick();
        }
        else
        {
            uint8_t program[] = {0xA9, accumlatorInitialValue, 0xE9, amountToSubtract};
            size_t n = sizeof(program) / sizeof(program[0]);
            bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        }

        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.a, 2) == hex(accumlatorInitialValue, 2));
        
        bus.cpu.tick();
        REQUIRE(bus.cpu.a == expectedValue);
    }
}

TEST_CASE("SBC Accumulator Correct When In BDC Mode", "[opcode][sbc]")
{
    auto [accumlatorInitialValue, amountToSubtract, setCarryFlag, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, bool, uint8_t>({
            {0x00, 0x99, false, 0x00},
            {0x00, 0x99, true, 0x01}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumlatorInitialValue, 2) << ", 0x" << hex(amountToSubtract, 2) << ", " << setCarryFlag << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        if (setCarryFlag)
        { 
            uint8_t program[] = {0x38, 0xF8, 0xA9, accumlatorInitialValue, 0xE9, amountToSubtract};
            size_t n = sizeof(program) / sizeof(program[0]);
            bus.cpu.LoadProgram(0x0000, program, n, 0x00);
            bus.cpu.tick();
        }
        else
        {
            uint8_t program[] = {0xF8, 0xA9, accumlatorInitialValue, 0xE9, amountToSubtract};
            size_t n = sizeof(program) / sizeof(program[0]);
            bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        }

        bus.cpu.tick();
        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.a, 2) == hex(accumlatorInitialValue, 2));
        
        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.a, 2) == hex(expectedValue, 2));
    }
}

TEST_CASE("SBC Overflow Correct When Not In BDC Mode", "[opcode][sbc]")
{
    auto [accumlatorInitialValue, amountToSubtract, CarryFlagSet, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, bool, uint8_t>({
            {0xFF, 0x01, false, false},
            {0xFF, 0x00, false, false},
            {0x80, 0x00, false, true},
            {0x80, 0x00, true, false},
            {0x81, 0x01, false, true},
            {0x81, 0x01, true, false},
            {0x00, 0x80, false, false},
            {0x00, 0x80, true, true},
            {0x01, 0x80, true, true},
            {0x01, 0x7F, false, false}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumlatorInitialValue, 2) << ", 0x" << hex(amountToSubtract, 2) << ", " << CarryFlagSet << ", " << expectedValue << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        if (CarryFlagSet)
        { 
            uint8_t program[] = {0x38, 0xA9, accumlatorInitialValue, 0xE9, amountToSubtract};
            size_t n = sizeof(program) / sizeof(program[0]);
            bus.cpu.LoadProgram(0x0000, program, n, 0x00);
            bus.cpu.tick();
        }
        else
        {
            uint8_t program[] = {0xA9, accumlatorInitialValue, 0xE9, amountToSubtract};
            size_t n = sizeof(program) / sizeof(program[0]);
            bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        }

        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.a, 2) == hex(accumlatorInitialValue, 2));
        
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.V) == 1) == expectedValue);
    }
}

TEST_CASE("SBC Overflow Correct When In BDC Mode", "[opcode][sbc]")
{
    auto [accumlatorInitialValue, amountToSubtract, CarryFlagSet, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, bool, uint8_t>({
            {0x63, 0x01, false, false},
            {0x63, 0x00, false, false},
            //{0, 1, false, true},
            //{1, 1, true, true},
            //{2, 1, true, false},
            //{1, 1, false, false}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(" << accumlatorInitialValue << ", " << amountToSubtract << ", " << CarryFlagSet << ", " << expectedValue << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        if (CarryFlagSet)
        { 
            uint8_t program[] = {0x38, 0xA9, accumlatorInitialValue, 0xE9, amountToSubtract};
            size_t n = sizeof(program) / sizeof(program[0]);
            bus.cpu.LoadProgram(0x0000, program, n, 0x00);
            bus.cpu.tick();
        }
        else
        {
            uint8_t program[] = {0xA9, accumlatorInitialValue, 0xE9, amountToSubtract};
            size_t n = sizeof(program) / sizeof(program[0]);
            bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        }

        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.a, 2) == hex(accumlatorInitialValue, 2));
        
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.V) == 1) == expectedValue);
    }
}

TEST_CASE("SBC Carry Correct", "[opcode][sbc]")
{
    auto [accumlatorInitialValue, amountToSubtract, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, bool>({
            {0x00, 0x00, false},
            {0x00, 0x01, false},
            {0x01, 0x00, true},
            {0x02, 0x01, true}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(" << accumlatorInitialValue << ", " << amountToSubtract << ", " << expectedValue << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        uint8_t program[] = {0xA9, accumlatorInitialValue, 0xE9, amountToSubtract};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.a, 2) == hex(accumlatorInitialValue, 2));
        
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.C) == 1) == expectedValue);
    }
}

TEST_CASE("SBC Zero Correct", "[opcode][sbc]")
{
    auto [accumlatorInitialValue, amountToSubtract, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, bool>({
            {0x00, 0x00, false},
            {0x00, 0x01, false},
            {0x01, 0x00, true},
            {0x01, 0x01, false}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumlatorInitialValue, 2) << ", 0x" << hex(amountToSubtract, 2) << ", " << expectedValue << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        uint8_t program[] = {0xA9, accumlatorInitialValue, 0xE9, amountToSubtract};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.a, 2) == hex(accumlatorInitialValue, 2));
        
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.Z) == 1) == expectedValue);
    }
}

TEST_CASE("SBC Negative Correct", "[opcode][sbc]")
{
    auto [accumlatorInitialValue, amountToSubtract, expectedValue] = 
        GENERATE( table<uint8_t, uint8_t, bool>({
            {0x80, 0x01, false},
            {0x81, 0x01, false},
            {0x00, 0x01, true},
            {0x01, 0x01, true}
        }));

    //Bus bus;

    DYNAMIC_SECTION("Check if XXX(0x" << hex(accumlatorInitialValue, 2) << ", 0x" << hex(amountToSubtract, 2) << ", " << expectedValue << ") works")
    {
        bus.cpu.reset();

        REQUIRE(bus.cpu.a == 0x00);

        uint8_t program[] = {0xA9, accumlatorInitialValue, 0xE9, amountToSubtract};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        REQUIRE(hex(bus.cpu.a, 2) == hex(accumlatorInitialValue, 2));
        
        bus.cpu.tick();
        REQUIRE((bus.cpu.GetFlag(bus.cpu.N) == 1) == expectedValue);
    }
}
#pragma endregion SBC - Subtraction With Borrow

#pragma region SEC - Set Carry Flag
TEST_CASE("SEC Carry Flag Set Correctly", "[opcode][sec]")
{
    bus.cpu.reset();

    uint8_t program[] = {0x38};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);

    bus.cpu.tick();
    REQUIRE(bus.cpu.GetFlag(bus.cpu.C) == true);
}
#pragma endregion SEC - Set Carry Flag

#pragma region SED - Set Decimal Mode
TEST_CASE("SED Decimal Mode Set Correctly", "[opcode][sed]")
{
    bus.cpu.reset();

    uint8_t program[] = {0xF8};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);

    bus.cpu.tick();
    REQUIRE(bus.cpu.GetFlag(bus.cpu.D) == true);
}
#pragma endregion SED - Set Decimal Mode

#pragma region SEI - Set Interrupt Flag
TEST_CASE("SEI Interrupt Flag Set Correctly", "[opcode][sei]")
{
    bus.cpu.reset();

    uint8_t program[] = {0x78};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);

    bus.cpu.tick();
    REQUIRE(bus.cpu.GetFlag(bus.cpu.I) == true);
}
#pragma endregion SEI - Set Interrupt Flag

#pragma region STA - Store Accumulator In Memory
TEST_CASE("STA Memory Has Correct Value", "[opcode][sta]")
{
    bus.cpu.reset();

    uint8_t program[] = {0xA9, 0x03, 0x85, 0x05};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);

    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE(hex(bus.read(0x05), 2) == hex(0x03, 2));
}
#pragma endregion STA - Store Accumulator In Memory

#pragma region STX - Set Memory To X
TEST_CASE("STX Memory Has Correct Value", "[opcode][stx]")
{
    bus.cpu.reset();

    uint8_t program[] = {0xA2, 0x03, 0x86, 0x05};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);

    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE(hex(bus.read(0x05), 2) == hex(0x03, 2));
}
#pragma endregion STX - Set Memory To X

#pragma region STY - Set Memory To Y
TEST_CASE("STY Memory Has Correct Value", "[opcode][sty]")
{
    bus.cpu.reset();

    uint8_t program[] = {0xA0, 0x03, 0x84, 0x05};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);

    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE(hex(bus.read(0x05), 2) == hex(0x03, 2));
}
#pragma endregion STY - Set Memory To Y

#pragma region TAX, TAY, TSX, TSY Tests
TEST_CASE("Transfer Correct Value Set", "[opcode][tax][tay][tsx][tsy]")
{
    auto [operation, transferFrom, transferTo] = 
        GENERATE( table<uint8_t, CPUTest::RegisterMode, CPUTest::RegisterMode>({
            {0xAA, CPUTest::Accumulator, CPUTest::XRegister},
            {0xA8, CPUTest::Accumulator, CPUTest::YRegister},
            {0x8A, CPUTest::XRegister, CPUTest::Accumulator},
            {0x98, CPUTest::YRegister, CPUTest::Accumulator}
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(operation, 2) << ", 0x" << hex(transferFrom, 2) << ", " << hex(transferTo, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t loadOperation;
        switch (transferFrom)
        {
            case CPUTest::Accumulator:
                loadOperation = 0xA9;
                break;
            case CPUTest::XRegister:
                loadOperation = 0xA2;
                break;
            case CPUTest::YRegister:
                loadOperation = 0xA0;
                break;
        }

        uint8_t program[] = {loadOperation, 0x03, operation};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        switch (transferFrom)
        {
            case CPUTest::Accumulator:
                REQUIRE(hex(bus.cpu.a, 2) == hex(0x03, 2));
                break;
            case CPUTest::XRegister:
                REQUIRE(hex(bus.cpu.x, 2) == hex(0x03, 2));
                break;
            case CPUTest::YRegister:
                REQUIRE(hex(bus.cpu.y, 2) == hex(0x03, 2));
                break;
        }


    }
}

TEST_CASE("Transfer Negative Value Set", "[opcode][tax][tay][tsx][tsy]")
{
    auto [operation, value, transferFrom, expectedResult] = GENERATE( table<uint8_t, uint8_t, CPUTest::RegisterMode, bool>({
        {0xAA, 0x80, CPUTest::Accumulator, true},
        {0xA8, 0x80, CPUTest::Accumulator, true},
        {0x8A, 0x80, CPUTest::XRegister, true},
        {0x98, 0x80, CPUTest::YRegister, true},
        {0xAA, 0xFF, CPUTest::Accumulator, true},
        {0xA8, 0xFF, CPUTest::Accumulator, true},
        {0x8A, 0xFF, CPUTest::XRegister, true},
        {0x98, 0xFF, CPUTest::YRegister, true},
        {0xAA, 0x7F, CPUTest::Accumulator, false},
        {0xA8, 0x7F, CPUTest::Accumulator, false},
        {0x8A, 0x7F, CPUTest::XRegister, false},
        {0x98, 0x7F, CPUTest::YRegister, false},
        {0xAA, 0x00, CPUTest::Accumulator, false},
        {0xA8, 0x00, CPUTest::Accumulator, false},
        {0x8A, 0x00, CPUTest::XRegister, false},
        {0x98, 0x00, CPUTest::YRegister, false},
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(operation, 2) << ", 0x" << hex(value, 2) << ", 0x" << hex(transferFrom, 2) << ", " << hex(expectedResult, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t loadOperation;
        switch (transferFrom)
        {
            case CPUTest::Accumulator:
                loadOperation = 0xA9;
                break;
            case CPUTest::XRegister:
                loadOperation = 0xA2;
                break;
            case CPUTest::YRegister:
                loadOperation = 0xA0;
                break;
        }

        uint8_t program[] = {loadOperation, value, operation};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.N) == expectedResult);
    }
}

TEST_CASE("Transfer Zero Value Set", "[opcode][tax][tay][tsx][tsy]")
{
    auto [operation, value, transferFrom, expectedResult] = GENERATE( table<uint8_t, uint8_t, CPUTest::RegisterMode, bool>({
        {0xAA, 0xFF, CPUTest::Accumulator, false},
        {0xA8, 0xFF, CPUTest::Accumulator, false},
        {0x8A, 0xFF, CPUTest::XRegister, false},
        {0x98, 0xFF, CPUTest::YRegister, false},
        {0xAA, 0x00, CPUTest::Accumulator, true},
        {0xA8, 0x00, CPUTest::Accumulator, true},
        {0x8A, 0x00, CPUTest::XRegister, true},
        {0x98, 0x00, CPUTest::YRegister, true},
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(operation, 2) << ", 0x" << hex(value, 2) << ", 0x" << hex(transferFrom, 2) << ", " << hex(expectedResult, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t loadOperation;
        switch (transferFrom)
        {
            case CPUTest::Accumulator:
                loadOperation = 0xA9;
                break;
            case CPUTest::XRegister:
                loadOperation = 0xA2;
                break;
            case CPUTest::YRegister:
                loadOperation = 0xA0;
                break;
        }

        uint8_t program[] = {loadOperation, value, operation};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.Z) == expectedResult);
    }
}
#pragma endregion TAX, TAY, TSX, TSY Tests

#pragma region TSX - Transfer Stack Pointer to X Register
TEST_CASE("TSX XRegister Set Correctly", "[opcode][tsx]")
{
    bus.cpu.reset();

    uint8_t program[] = {0xBA};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);

    uint8_t stackLocation = bus.cpu.stkp;

    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE(hex(bus.cpu.x, 2) == hex(stackLocation, 2));
}

TEST_CASE("TSX Negative Set Correctly", "[opcode][tsx]")
{
    auto [valueToLoad, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x00, false},
            {0x7F, false},
            {0x80, true},
            {0xFF, true},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(valueToLoad, 2) << ", " << expectedValue << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA2, valueToLoad, 0x9A, 0xBA};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.N) == expectedValue);
    }
}

TEST_CASE("TSX Zero Set Correctly", "[opcode][tsx]")
{
    auto [valueToLoad, expectedValue] = 
        GENERATE( table<uint8_t, bool>({
            {0x00, true},
            {0x01, false},
            {0xFF, false},
        })
    );

    //Bus bus

    DYNAMIC_SECTION("Check if XXX(0x" << hex(valueToLoad, 2) << ", " << expectedValue << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA2, valueToLoad, 0x9A, 0xBA};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.GetFlag(bus.cpu.Z) == expectedValue);
    }
}
#pragma endregion TSX - Transfer Stack Pointer to X Register

#pragma region TXS - Transfer X Register to Stack Pointer
TEST_CASE("TXS Stack Pointer Set Correctly", "[opcode][txs]")
{
    bus.cpu.reset();

    uint8_t program[] = {0xA2, 0xAA, 0x9A};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x00, program, n, 0x00);

    bus.cpu.tick();
    bus.cpu.tick();

    REQUIRE(hex(bus.cpu.stkp, 2) == hex(0xAA, 2));
}
#pragma endregion TXS - Transfer X Register to Stack Pointer

#pragma region Accumulator Address Tests
TEST_CASE("Immediate Mode Accumulator Has Correct Result", "[index][acc]")
{
    auto [operation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
        {0x69, 0x01, 0x01, 0x02}, // ADC
        {0x29, 0x03, 0x03, 0x03}, // AND
        {0xA9, 0x04, 0x03, 0x03}, // LDA
        {0x49, 0x55, 0xAA, 0xFF}, // EOR
        {0x09, 0x55, 0xAA, 0xFF}, // ORA
        {0xE9, 0x03, 0x01, 0x01}, // SBC
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", 0x" << hex(accumulatorInitialValue, 2) << ", 0x" << hex(valueToTest, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorInitialValue, operation, valueToTest};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.a == expectedValue);
    }
}

TEST_CASE("ZeroPage Mode Accumulator Has Correct Result", "[index][acc]")
{
    auto [operation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
        {0x65, 0x01, 0x01, 0x02}, // ADC
        {0x25, 0x03, 0x03, 0x03}, // AND
        {0xA5, 0x04, 0x03, 0x03}, // LDA
        {0x45, 0x55, 0xAA, 0xFF}, // EOR
        {0x05, 0x55, 0xAA, 0xFF}, // ORA
        {0xE5, 0x03, 0x01, 0x01}, // SBC
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", 0x" << hex(accumulatorInitialValue, 2) << ", 0x" << hex(valueToTest, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorInitialValue, operation, 0x05, 0x00, valueToTest};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.a == expectedValue);
    }
}

TEST_CASE("ZeroPageX Mode Accumulator Has Correct Result", "[index][acc]")
{
    auto [operation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
        {0x75, 0x00, 0x03, 0x03}, // ADC
        {0x35, 0x03, 0x03, 0x03}, // AND
        {0xB5, 0x04, 0x03, 0x03}, // LDA
        {0x55, 0x55, 0xAA, 0xFF}, // EOR
        {0x15, 0x55, 0xAA, 0xFF}, // ORA
        {0xF5, 0x03, 0x01, 0x01}, // SBC
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", 0x" << hex(accumulatorInitialValue, 2) << ", 0x" << hex(valueToTest, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorInitialValue, 0xA2, 0x01, operation, 0x06, 0x00, valueToTest};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.a == expectedValue);
    }
}

TEST_CASE("Absolute Mode Accumulator Has Correct Result", "[index][acc]")
{
    auto [operation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
        {0x6D, 0x00, 0x03, 0x03}, // ADC
        {0x2D, 0x03, 0x03, 0x03}, // AND
        {0xAD, 0x04, 0x03, 0x03}, // LDA
        {0x4D, 0x55, 0xAA, 0xFF}, // EOR
        {0x0D, 0x55, 0xAA, 0xFF}, // ORA
        {0xED, 0x03, 0x01, 0x01}, // SBC
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", 0x" << hex(accumulatorInitialValue, 2) << ", 0x" << hex(valueToTest, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorInitialValue, operation, 0x06, 0x00, 0x00, valueToTest};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.a == expectedValue);
    }
}

TEST_CASE("AbsoluteX Mode Accumulator Has Correct Result", "[index][acc]")
{
    auto [operation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
        {0x7D, 0x01, 0x01, 0x02}, // ADC
        {0x3D, 0x03, 0x03, 0x03}, // AND
        {0xBD, 0x04, 0x03, 0x03}, // LDA
        {0x5D, 0x55, 0xAA, 0xFF}, // EOR
        {0x1D, 0x55, 0xAA, 0xFF}, // ORA
        {0xFD, 0x03, 0x01, 0x01}, // SBC
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", 0x" << hex(accumulatorInitialValue, 2) << ", 0x" << hex(valueToTest, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[]= {0xA9, accumulatorInitialValue, 0xA2, 0x09, operation, 0xff, 0xff, 0x00, valueToTest};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.a == expectedValue);
    }
}

TEST_CASE("AbsoluteX Mode Accumulator Has Correct Result When Wrapped", "[index][acc]")
{
    auto [operation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
        {0x7D, 0x01, 0x01, 0x02}, // ADC
        {0x3D, 0x03, 0x03, 0x03}, // AND
        {0xBD, 0x04, 0x03, 0x03}, // LDA
        {0x5D, 0x55, 0xAA, 0xFF}, // EOR
        {0x1D, 0x55, 0xAA, 0xFF}, // ORA
        {0xFD, 0x03, 0x01, 0x01}, // SBC
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", 0x" << hex(accumulatorInitialValue, 2) << ", 0x" << hex(valueToTest, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorInitialValue, 0xA2, 0x01, operation, 0x07, 0x00, 0x00, valueToTest};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.a == expectedValue);
    }
}

TEST_CASE("AbsoluteY Mode Accumulator Has Correct Result", "[index][acc]")
{
    auto [operation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
        {0x79, 0x01, 0x01, 0x02}, // ADC
        {0x39, 0x03, 0x03, 0x03}, // AND
        {0xB9, 0x04, 0x03, 0x03}, // LDA
        {0x59, 0x55, 0xAA, 0xFF}, // EOR
        {0x19, 0x55, 0xAA, 0xFF}, // ORA
        {0xF9, 0x03, 0x01, 0x01}, // SBC
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", 0x" << hex(accumulatorInitialValue, 2) << ", 0x" << hex(valueToTest, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[]= {0xA9, accumulatorInitialValue, 0xA0, 0x01, operation, 0x07, 0x00, 0x00, valueToTest};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.a == expectedValue);
    }
}

TEST_CASE("AbsoluteY Mode Accumulator Has Correct Result When Wrapped", "[index][acc]")
{
    auto [operation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
        {0x79, 0x01, 0x01, 0x02}, // ADC
        {0x39, 0x03, 0x03, 0x03}, // AND
        {0xB9, 0x04, 0x03, 0x03}, // LDA
        {0x59, 0x55, 0xAA, 0xFF}, // EOR
        {0x19, 0x55, 0xAA, 0xFF}, // ORA
        {0xF9, 0x03, 0x01, 0x01}, // SBC
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", 0x" << hex(accumulatorInitialValue, 2) << ", 0x" << hex(valueToTest, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorInitialValue, 0xA0, 0x09, operation, 0xff, 0xff, 0x00, valueToTest};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.a == expectedValue);
    }
}

TEST_CASE("Indexed Indirect Mode Accumulator Has Correct Result", "[index][acc]")
{
    auto [operation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
        {0x61, 0x01, 0x01, 0x02}, // ADC
        {0x21, 0x03, 0x03, 0x03}, // AND
        {0xA1, 0x04, 0x03, 0x03}, // LDA
        {0x41, 0x55, 0xAA, 0xFF}, // EOR
        {0x01, 0x55, 0xAA, 0xFF}, // ORA
        {0xE1, 0x03, 0x01, 0x01}, // SBC
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", 0x" << hex(accumulatorInitialValue, 2) << ", 0x" << hex(valueToTest, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[]= {0xA9, accumulatorInitialValue, 0xA6, 0x06, operation, 0x01, 0x06, 0x9, 0x00, valueToTest};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.a == expectedValue);
    }
}

TEST_CASE("Indexed Indirect Mode Accumulator Has Correct Result When Wrapped", "[index][acc]")
{
    auto [operation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
        {0x61, 0x01, 0x01, 0x02}, // ADC
        {0x21, 0x03, 0x03, 0x03}, // AND
        {0xA1, 0x04, 0x03, 0x03}, // LDA
        {0x41, 0x55, 0xAA, 0xFF}, // EOR
        {0x01, 0x55, 0xAA, 0xFF}, // ORA
        {0xE1, 0x03, 0x01, 0x01}, // SBC
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", 0x" << hex(accumulatorInitialValue, 2) << ", 0x" << hex(valueToTest, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorInitialValue, 0xA6, 0x06, operation, 0xff, 0x08, 0x9, 0x00, valueToTest};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.a == expectedValue);
    }
}

TEST_CASE("Indirect Indexed Mode Accumulator Has Correct Result", "[index][acc]")
{
    auto [operation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
        {0x71, 0x01, 0x01, 0x02}, // ADC
        {0x31, 0x03, 0x03, 0x03}, // AND
        {0xB1, 0x04, 0x03, 0x03}, // LDA
        {0x51, 0x55, 0xAA, 0xFF}, // EOR
        {0x11, 0x55, 0xAA, 0xFF}, // ORA
        {0xF1, 0x03, 0x01, 0x01}, // SBC
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", 0x" << hex(accumulatorInitialValue, 2) << ", 0x" << hex(valueToTest, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[]= {0xA9, accumulatorInitialValue, 0xA0, 0x01, operation, 0x07, 0x00, 0x08, 0x00, valueToTest};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.a == expectedValue);
    }
}

TEST_CASE("Indirect Indexed Mode Accumulator Has Correct Result When Wrapped", "[index][acc]")
{
    auto [operation, accumulatorInitialValue, valueToTest, expectedValue] = GENERATE( table<uint8_t, uint8_t, uint8_t, uint8_t>({
        {0x71, 0x01, 0x01, 0x02}, // ADC
        {0x31, 0x03, 0x03, 0x03}, // AND
        {0xB1, 0x04, 0x03, 0x03}, // LDA
        {0x51, 0x55, 0xAA, 0xFF}, // EOR
        {0x11, 0x55, 0xAA, 0xFF}, // ORA
        {0xF1, 0x03, 0x01, 0x01}, // SBC
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", 0x" << hex(accumulatorInitialValue, 2) << ", 0x" << hex(valueToTest, 2) << ", 0x" << hex(expectedValue, 2) << ") works")
    {
        bus.cpu.reset();

        uint8_t program[] = {0xA9, accumulatorInitialValue, 0xA0, 0x0A, operation, 0x07, 0x00, 0xFF, 0xFF, valueToTest};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();
        bus.cpu.tick();

        REQUIRE(bus.cpu.a == expectedValue);
    }
}
#pragma endregion Accumulator Address Tests

#pragma region Index Address Tests
TEST_CASE("ZeroPage Mode Index Has Correct Result", "[index][addressmode]")
{
    auto [operation, valueToLoad, testXRegister] = GENERATE( table<uint8_t, uint8_t, bool>({
        {0xA6, 0x03, true},  // LDX Zero Page
        {0xB6, 0x03, true},  // LDX Zero Page Y
        {0xA4, 0x03, false}, // LDY Zero Page
        {0xB4, 0x03, false}, // LDY Zero Page X
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", 0x" << hex(valueToLoad, 2) << ", " << testXRegister << ") works")
    {
        bus.cpu.reset();

        uint8_t program[]= {operation, 0x03, 0x00, valueToLoad};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();

        if (testXRegister)
        {
            REQUIRE(bus.cpu.x == valueToLoad);
        }
        else
        {
            REQUIRE(bus.cpu.y == valueToLoad);
        }
    }
}

TEST_CASE("ZeroPageX Mode Index Has Correct Result When Wrapped", "[index][addressmode]")
{
    auto [operation, valueToLoad, testXRegister] = GENERATE( table<uint8_t, uint8_t, bool>({
        {0xB6, 0x03, true},  // LDX Zero Page Y
        {0xB4, 0x03, false}, // LDY Zero Page X
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", 0x" << hex(valueToLoad, 2) << ", " << testXRegister << ") works")
    {
        bus.cpu.reset();

        uint8_t XRegister = testXRegister ? 0xA0 : 0xA2;

        uint8_t program[]= {XRegister, 0xFF, operation, 0x06, 0x00, valueToLoad};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        if (testXRegister)
        {
            REQUIRE(bus.cpu.x == valueToLoad);
        }
        else
        {
            REQUIRE(bus.cpu.y == valueToLoad);
        }
    }
}

TEST_CASE("Absolute Mode Index Has Correct Result", "[index][addressmode]")
{
    auto [operation, valueToLoad, testXRegister] = GENERATE( table<uint8_t, uint8_t, bool>({
        {0xAE, 0x03, true},  // LDX Absolute
        {0xAC, 0x03, false}, // LDY Absolute
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", 0x" << hex(valueToLoad, 2) << ", " << testXRegister << ") works")
    {
        bus.cpu.reset();

        uint8_t program[]= {operation, 0x04, 0x00, 0x00, valueToLoad};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();

        if (testXRegister)
        {
            REQUIRE(hex(bus.cpu.x, 2) == hex(valueToLoad, 2));
        }
        else
        {
            REQUIRE(hex(bus.cpu.y, 2) == hex(valueToLoad, 2));
        }
    }
}
#pragma endregion Index Address Tests

#pragma region Compare Address Tests
#pragma endregion Compare Address Tests

#pragma region Decrement/Increment Address Tests
#pragma endregion Decrement/Increment Address Tests

#pragma region Store In Memory Address Tests
#pragma endregion Store In Memory Address Tests

#pragma region Cycle Tests
TEST_CASE("NumberOfCyclesRemaining Correct After Operations That Do Not Wrap", "[counter][cycles]")
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
        {0x1E, 7}, // ASL Absolute X
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
        {0x5E, 7}, // LSR Absolute X
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
        {0x3E, 7}, // ROL Absolute X
        {0x6A, 2}, // ROR Accumulator
        {0x66, 5}, // ROR Zero Page
        {0x76, 6}, // ROR Zero Page X
        {0x6E, 6}, // ROR Absolute
        {0x7E, 7}, // ROR Absolute X
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
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", " << numberOfCyclesUsed << ") works")
    {
        bus.cpu.reset();

        uint8_t program[]= {operation, 0x00};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        uint8_t startingNumberOfCycles = bus.cpu.cycle_count;

        //printf("startingNumberOfCycles: %d\n", startingNumberOfCycles);

        bus.cpu.tick();

        //                      EXPECTED                            ACTUAL
        REQUIRE(startingNumberOfCycles + numberOfCyclesUsed == bus.cpu.cycle_count);
    }
}

TEST_CASE("NumberOfCyclesRemaining Correct When In AbsoluteX And Wrap", "[counter][cycles]")
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

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", " << numberOfCyclesUsed << ") works")
    {
        bus.cpu.reset();

        uint8_t program[]= {0xA6, 0x06, operation, 0xff, 0xff, 0x00, 0x03};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();

        //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
        uint8_t startingNumberOfCycles = bus.cpu.cycle_count;
        bus.cpu.tick();

        REQUIRE(startingNumberOfCycles + numberOfCyclesUsed == bus.cpu.cycle_count);
    }
}

TEST_CASE("NumberOfCyclesRemaining Correct When In AbsoluteY And Wrap", "[counter][cycles]")
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

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", " << numberOfCyclesUsed << ") works")
    {
        bus.cpu.reset();

        uint8_t program[]= {0xA4, 0x06, operation, 0xff, 0xff, 0x00, 0x03};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        bus.cpu.tick();

        //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
        uint8_t startingNumberOfCycles = bus.cpu.cycle_count;
        bus.cpu.tick();

        REQUIRE(startingNumberOfCycles + numberOfCyclesUsed == bus.cpu.cycle_count);
    }
}

TEST_CASE("NumberOfCyclesRemaining Correct When In IndirectIndexed And Wrap", "[counter][cycles]")
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

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", " << numberOfCyclesUsed << ") works")
    {
        bus.cpu.reset();

        uint8_t program[]= {0xA0, 0x04, operation, 0x05, 0x08, 0xFF, 0xFF, 0x03};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        bus.cpu.tick();

        //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
        uint8_t startingNumberOfCycles = bus.cpu.cycle_count;
        bus.cpu.tick();

        REQUIRE(startingNumberOfCycles + numberOfCyclesUsed == bus.cpu.cycle_count);
    }
}

TEST_CASE("NumberOfCyclesRemaining Correct When Relative And Branch On Carry Set", "[counter][cycles]")
{
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
        {0x90, 2}, // BCC
        {0xB0, 3}, // BCS
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", " << numberOfCyclesUsed << ") works")
    {
        bus.cpu.reset();

        uint8_t program[]= {0x38, operation, 0x00};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();

        //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
        uint8_t startingNumberOfCycles = bus.cpu.cycle_count;
        bus.cpu.tick();

        REQUIRE(startingNumberOfCycles + numberOfCyclesUsed == bus.cpu.cycle_count);
    }
}

TEST_CASE("NumberOfCyclesRemaining Correct When Relative And Branch On Carry Clear", "[counter][cycles]")
{
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
        {0x90, 3}, // BCC
        {0xB0, 2}, // BCS
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", " << numberOfCyclesUsed << ") works")
    {
        bus.cpu.reset();

        uint8_t program[]= {0x18, operation, 0x00};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();

        //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
        uint8_t startingNumberOfCycles = bus.cpu.cycle_count;
        bus.cpu.tick();

        REQUIRE(startingNumberOfCycles + numberOfCyclesUsed == bus.cpu.cycle_count);
    }
}

TEST_CASE("NumberOfCyclesRemaining Correct When Relative And Branch On Carry And Wrap", "[counter][cycles]")
{
    auto [operation, numberOfCyclesUsed, isCarrySet, wrapRight] = GENERATE( table<uint8_t, uint8_t, bool, bool>({
        {0x90, 4, false, true},  //BCC
        {0x90, 4, false, false}, //BCC
        {0xB0, 4, true, true},   //BCC
        {0xB0, 4, true, false},  //BCC
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", " << numberOfCyclesUsed << ") works")
    {
        bus.cpu.reset();

        uint8_t carryOperation = isCarrySet ? 0x38 : 0x18;
        uint16_t initialAddress = wrapRight ? 0xFFF0 : 0x00;
        uint8_t amountToMove = wrapRight ? 0x0F : 0x84;

        uint8_t program[]= {carryOperation, operation, amountToMove, 0x00};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(initialAddress, program, n, initialAddress);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();

        //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
        uint8_t startingNumberOfCycles = bus.cpu.cycle_count;
        bus.cpu.tick();

        REQUIRE(startingNumberOfCycles + numberOfCyclesUsed == bus.cpu.cycle_count);
    }
}

TEST_CASE("NumberOfCyclesRemaining Correct When Relative And Branch On Zero Set", "[counter][cycles]")
{
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
        {0xF0, 3}, // BEQ
        {0xD0, 2}, // BNE
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", " << numberOfCyclesUsed << ") works")
    {
        bus.cpu.reset();

        uint8_t program[]= {0xA9, 0x00, operation, 0x00};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();

        //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
        uint8_t startingNumberOfCycles = bus.cpu.cycle_count;
        bus.cpu.tick();

        REQUIRE(startingNumberOfCycles + numberOfCyclesUsed == bus.cpu.cycle_count);
    }
}

TEST_CASE("NumberOfCyclesRemaining Correct When Relative And Branch On Zero Clear", "[counter][cycles]")
{
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
        {0x90, 3}, // BEQ
        {0xB0, 2}, // BNE
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", " << numberOfCyclesUsed << ") works")
    {
        bus.cpu.reset();

        uint8_t program[]= {0xA9, 0x01, operation, 0x00};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();

        //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
        uint8_t startingNumberOfCycles = bus.cpu.cycle_count;
        bus.cpu.tick();

        REQUIRE(startingNumberOfCycles + numberOfCyclesUsed == bus.cpu.cycle_count);
    }
}

TEST_CASE("NumberOfCyclesRemaining Correct When Relative And Branch On Zero And Wrap", "[counter][cycles]")
{
    auto [operation, numberOfCyclesUsed, isZeroSet, wrapRight] = GENERATE( table<uint8_t, uint8_t, bool, bool>({
        {0xF0, 4, true, true},  //BEQ
        {0xF0, 4, true, false}, //BEQ
        {0xD0, 4, false, true},   //BNE
        {0xD0, 4, false, false},  //BNE
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", " << numberOfCyclesUsed << ") works")
    {
        bus.cpu.reset();

        uint8_t newAccumulatorValue = isZeroSet ? 0x00 : 0x01;
        uint16_t initialAddress = wrapRight ? 0xFFF0 : 0x00;
        uint8_t amountToMove = wrapRight ? 0x0D : 0x84;

        uint8_t program[]= {0xA9, newAccumulatorValue, operation, amountToMove, 0x00};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(initialAddress, program, n, initialAddress);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();

        //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
        uint8_t startingNumberOfCycles = bus.cpu.cycle_count;
        bus.cpu.tick();

        REQUIRE(startingNumberOfCycles + numberOfCyclesUsed == bus.cpu.cycle_count);
    }
}

TEST_CASE("NumberOfCyclesRemaining Correct When Relative And Branch On Negative Set", "[counter][cycles]")
{
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
        {0x30, 3}, // BEQ
        {0x10, 2}, // BNE
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", " << numberOfCyclesUsed << ") works")
    {
        bus.cpu.reset();

        uint8_t program[]= {0xA9, 0x80, operation, 0x00};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();

        //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
        uint8_t startingNumberOfCycles = bus.cpu.cycle_count;
        bus.cpu.tick();

        REQUIRE(startingNumberOfCycles + numberOfCyclesUsed == bus.cpu.cycle_count);
    }
}

TEST_CASE("NumberOfCyclesRemaining Correct When Relative And Branch On Negative Clear", "[counter][cycles]")
{
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
        {0x30, 2}, // BEQ
        {0x10, 3}, // BNE
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", " << numberOfCyclesUsed << ") works")
    {
        bus.cpu.reset();

        uint8_t program[]= {0xA9, 0x79, operation, 0x00};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();

        //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
        uint8_t startingNumberOfCycles = bus.cpu.cycle_count;
        bus.cpu.tick();

        REQUIRE(startingNumberOfCycles + numberOfCyclesUsed == bus.cpu.cycle_count);
    }
}

TEST_CASE("NumberOfCyclesRemaining Correct When Relative And Branch On Negative And Wrap", "[counter][cycles]")
{
    auto [operation, numberOfCyclesUsed, isNegativeSet, wrapRight] = GENERATE( table<uint8_t, uint8_t, bool, bool>({
        {0x30, 4, true, true},  //BEQ
        {0x30, 4, true, false}, //BEQ
        {0x10, 4, false, true},   //BNE
        {0x10, 4, false, false},  //BNE
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", " << numberOfCyclesUsed << ") works")
    {
        bus.cpu.reset();

        uint8_t newAccumulatorValue = isNegativeSet ? 0x80 : 0x79;
        uint16_t initialAddress = wrapRight ? 0xFFF0 : 0x00;
        uint8_t amountToMove = wrapRight ? 0x0D : 0x84;

        uint8_t program[]= {0xA9, newAccumulatorValue, operation, amountToMove, 0x00};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(initialAddress, program, n, initialAddress);
        REQUIRE(bus.cpu.a == 0x00);
        bus.cpu.tick();

        //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
        uint8_t startingNumberOfCycles = bus.cpu.cycle_count;
        bus.cpu.tick();

        REQUIRE(startingNumberOfCycles + numberOfCyclesUsed == bus.cpu.cycle_count);
    }
}

TEST_CASE("NumberOfCyclesRemaining Correct When Relative And Branch On Overflow Set", "[counter][cycles]")
{
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
        {0x50, 2}, // BVC
        {0x70, 3}, // BVS
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", " << numberOfCyclesUsed << ") works")
    {
        bus.cpu.reset();

        uint8_t program[]= {0xA9, 0x01, 0x69, 0x7F, operation, 0x00};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
        uint8_t startingNumberOfCycles = bus.cpu.cycle_count;
        bus.cpu.tick();

        REQUIRE(startingNumberOfCycles + numberOfCyclesUsed == bus.cpu.cycle_count);
    }
}

TEST_CASE("NumberOfCyclesRemaining Correct When Relative And Branch On Overflow Clear", "[counter][cycles]")
{
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
        {0x50, 3}, // BVC
        {0x70, 2}, // BVS
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", " << numberOfCyclesUsed << ") works")
    {
        bus.cpu.reset();

        uint8_t program[]= {0xA9, 0x01, 0x69, 0x01, operation, 0x00};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);
        REQUIRE(bus.cpu.a == 0x00);

        bus.cpu.tick();
        bus.cpu.tick();

        //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
        uint8_t startingNumberOfCycles = bus.cpu.cycle_count;
        bus.cpu.tick();

        REQUIRE(startingNumberOfCycles + numberOfCyclesUsed == bus.cpu.cycle_count);
    }
}

TEST_CASE("NumberOfCyclesRemaining Correct When Relative And Branch On Overflow And Wrap", "[counter][cycles]")
{
    auto [operation, numberOfCyclesUsed, isOverflowSet, wrapRight] = GENERATE( table<uint8_t, uint8_t, bool, bool>({
        {0x50, 4, false, true},  //BVC
        {0x50, 4, false, false}, //BVC
        {0x70, 4, true, true},   //BVS
        {0x70, 4, true, false},  //BVS
    }));

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", " << numberOfCyclesUsed <<  ", " << isOverflowSet << ", " << wrapRight << ") works")
    {
        bus.cpu.reset();

        uint8_t newAccumulatorValue = isOverflowSet ? 0x7F : 0x00;
        uint16_t initialAddress = wrapRight ? 0xFFF0 : 0x00;
        uint8_t amountToMove = wrapRight ? 0x0B : 0x86;

        uint8_t program[]= {0xA9, newAccumulatorValue, 0x69, 0x01, operation, amountToMove, 0x00};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(initialAddress, program, n, initialAddress);

        bus.cpu.tick();
        bus.cpu.tick();

        //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
        uint8_t startingNumberOfCycles = bus.cpu.cycle_count;
        bus.cpu.tick();

        REQUIRE(startingNumberOfCycles + numberOfCyclesUsed == bus.cpu.cycle_count);
    }
}

TEST_CASE("NumberOfCyclesRemaining Correct After NOP Operations", "[counter][cycles]")
{
    auto [operation, numberOfCyclesUsed] = GENERATE( table<uint8_t, uint8_t>({
        {0xEA, 2}, // NOP Implied
#ifdef ILLEGAL
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

    //Bus bus

    DYNAMIC_SECTION("Check if " << bus.cpu.inst.getInstructionName(operation) << "(0x" << hex(operation, 2) << ", " << numberOfCyclesUsed << ") works")
    {
        bus.cpu.reset();

        uint8_t program[]= {operation, 0x00};
        size_t n = sizeof(program) / sizeof(program[0]);
        bus.cpu.LoadProgram(0x0000, program, n, 0x00);

        uint8_t startingNumberOfCycles = bus.cpu.cycle_count;

        //printf("startingNumberOfCycles: %d\n", startingNumberOfCycles);

        bus.cpu.tick();

        //                      EXPECTED                            ACTUAL
        REQUIRE(startingNumberOfCycles + numberOfCyclesUsed == bus.cpu.cycle_count);
    }
}

#pragma endregion Cycle Tests

#pragma region Program Counter Tests
TEST_CASE("Branch On Negative Set Program Counter Correct When NoBranch Occurs", "[counter][pc]")
{
    //Bus bus
    bus.cpu.reset();

    uint8_t program[]= {0xA9, 0x80, 0x10};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);
    REQUIRE(bus.cpu.a == 0x00);

    //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
    bus.cpu.tick();
    uint16_t currentProgramCounter = bus.cpu.pc;
    bus.cpu.tick();

    REQUIRE(hex(currentProgramCounter + 2, 4) == hex(bus.cpu.pc, 4));
}

TEST_CASE("Branch On Negative Clear Program Counter Correct When NoBranch Occurs", "[counter][pc]")
{
    //Bus bus
    bus.cpu.reset();

    uint8_t program[]= {0xA9, 0x79, 0x30};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);
    REQUIRE(bus.cpu.a == 0x00);

    //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
    bus.cpu.tick();
    uint8_t currentProgramCounter = bus.cpu.pc;
    bus.cpu.tick();

    REQUIRE(hex(currentProgramCounter + 2, 4) == hex(bus.cpu.pc, 4));
}

TEST_CASE("Branch On Overflow Set Program Counter Correct When NoBranch Occurs", "[counter][pc]")
{
    //Bus bus
    bus.cpu.reset();

    uint8_t program[]= {0xA9, 0x01, 0x69, 0x01, 0x70, 0x00};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);
    REQUIRE(bus.cpu.pc == 0x0000);

    //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
    bus.cpu.tick();
    bus.cpu.tick();
    uint16_t currentProgramCounter = bus.cpu.pc;

    bus.cpu.tick();

    REQUIRE(hex(currentProgramCounter + 0x2, 4) == hex(bus.cpu.pc, 4));
}

TEST_CASE("Branch On Overflow Clear Program Counter Correct When NoBranch Occurs", "[counter][pc]")
{
    //Bus bus
    bus.cpu.reset();

    uint8_t program[]= {0xA9, 0x01, 0x69, 0x7F, 0x50, 0x00};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0x0000, program, n, 0x00);
    REQUIRE(bus.cpu.pc == 0x0000);

    //Get the number of cycles after the register has been loaded, so we can isolate the operation under test
    bus.cpu.tick();
    bus.cpu.tick();
    uint16_t currentProgramCounter = bus.cpu.pc;

    bus.cpu.tick();

    REQUIRE(hex(currentProgramCounter + 0x2, 4) == hex(bus.cpu.pc, 4));
}

TEST_CASE("Program Counter Wraps Correctly", "[counter][pc]")
{
    //Bus bus
    bus.cpu.reset();

    REQUIRE(hex(bus.cpu.pc, 4) == hex(0x0000, 4));

    uint8_t program[] = {0x38};
    size_t n = sizeof(program) / sizeof(program[0]);
    bus.cpu.LoadProgram(0xFFFF, program, n, 0xFFFF);

    REQUIRE(hex(bus.cpu.pc, 4) == hex(0xFFFF, 4));

    bus.cpu.tick();

    REQUIRE(hex(bus.cpu.pc, 4) == hex(0x0000, 4));
}
#pragma endregion Program Counter Tests


