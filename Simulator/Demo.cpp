#include "Bus.hpp"
#include "Processor.hpp"

#include <iostream>
#include <sstream>

int main()
{
  CPU::Bus bus;
  
  bus.cpu.reset();

  // Load Program (assembled at https://www.masswerk.at/6502/assembler.html)
  /*
    *=$8000
    LDX #10
    STX $0000
    LDX #3
    STX $0001
    LDY $0000
    LDA #0
    CLC
    loop
    ADC $0001
    DEY
    BNE loop
    STA $0002
    NOP
    NOP
    NOP
  */
  uint8_t program[] = {
    0xA2, 0x0A, 0x8E, 0x00, 0x00, 0xA2, 0x03, 0x8E, 0x01, 0x00, 0xAC, 0x00, 0x00, 0xA9,
    0x00, 0x18, 0x6D, 0x01, 0x00, 0x88, 0xD0, 0xFA, 0x8D, 0x02, 0x00, 0xEA, 0xEA, 0xEA
  };
  size_t n = sizeof(program) / sizeof(program[0]);
  bus.cpu.LoadProgram(0x8000, program, n, 0x8000);

  for (uint8_t i = 0; i <= 30; i++)
  {
    bus.cpu.tick();
  }

  return 0;
}