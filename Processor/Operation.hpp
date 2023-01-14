#pragma once

#include <stdint.h>
//#include "Executioner.hpp"
#include "Instructions/InstructionTable.hpp"

namespace CPU
{
  namespace Instructions {
    class AddressMode;
    class Instruction;
    class InstructionTable;
  };

  class Executioner;
  class Operation
  {
  public:
    Operation(uint8_t op);
    Operation(CPU::Instructions::InstructionTable::OperationType operation);
    ~Operation() = default;

  private:
    uint8_t addressMode = 0x00;
    uint8_t instruction = 0x00;
    uint8_t cycles = 0x00;

  public:
    static Operation* get();
    static Operation* get(uint8_t* op);
    static Operation* get(uint8_t op);

    uint8_t execute(CPU::Executioner* e);
    uint8_t execute(CPU::Executioner* e, CPU::Executioner::ExecutionType operation);

  public:
    uint8_t getAddressMode();
    CPU::Executioner::ExecutionType getAddressOp();
    const char* getAddressModeName();

    uint8_t getInstruction();
    CPU::Executioner::ExecutionType getInstructionOp();
    const char* getInstructionName();

    uint8_t getOperationCycles();

  };
};
