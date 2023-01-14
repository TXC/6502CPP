#include "Operation.hpp"
//#include "Executioner.hpp"
#include "Instructions/AddressMode.hpp"
#include "Instructions/Instruction.hpp"
//#include "Instructions/InstructionTable.hpp"
#include <stdexcept>
#include <cstdint>

namespace IN = CPU::Instructions;
namespace CPU
{
  Operation::Operation(uint8_t op)
  {
    IN::InstructionTable* tableObj = &IN::InstructionTable::getInstance();
    if (tableObj->exists(op) == false) {
      throw std::invalid_argument("Invalid operation");
    }

    auto opType = tableObj->get(op);
    instruction = opType.operate;
    addressMode = opType.addrmode;
    cycles = opType.cycles;
  };
  Operation::Operation(IN::InstructionTable::OperationType op)
  {
    instruction = op.operate;
    addressMode = op.addrmode;
    cycles = op.cycles;
  };

  Operation* Operation::get(uint8_t* op)
  {
    return get(op);
  }
  Operation* Operation::get(uint8_t op)
  {
    IN::InstructionTable* tableObj = &IN::InstructionTable::getInstance();
    if (tableObj->exists(op) == false) {
      throw std::invalid_argument("Invalid operation");
    }

    auto opType = tableObj->get(op);
    Operation* opObj = new Operation(opType);
    return opObj;
  }

  uint8_t Operation::getAddressMode()
  {
    return addressMode;
  }
  Executioner::ExecutionType Operation::getAddressOp()
  {
    IN::AddressMode* addrObj = &IN::AddressMode::getInstance();
    return addrObj->get(addressMode).op;
  }
  const char* Operation::getAddressModeName()
  {
    IN::AddressMode* addrObj = &IN::AddressMode::getInstance();
    return addrObj->get(addressMode).name.c_str();
  }

  uint8_t Operation::getInstruction()
  {
    return instruction;
  }
  Executioner::ExecutionType Operation::getInstructionOp()
  {
    IN::Instruction* instObj = &IN::Instruction::getInstance();
    return instObj->get(instruction).op;
  }
  const char* Operation::getInstructionName()
  {
    IN::Instruction* instObj = &IN::Instruction::getInstance();
    return instObj->get(instruction).name.c_str();
  }

  uint8_t Operation::getOperationCycles()
  {
    IN::InstructionTable* instTableObj = &IN::InstructionTable::getInstance();
    return instTableObj->get(instruction).cycles;
  }

  // Execute full instruction (address mode and instruction)
  // Returns no. of cycles instruction shall take
  uint8_t Operation::execute(Executioner* e)
  {
    IN::AddressMode* addrObj = &IN::AddressMode::getInstance();
    IN::Instruction* instObj = &IN::Instruction::getInstance();

    //ExecutionType _addrMode = AddressMode[lookup[addressMode].addrmode].op;
    //ExecutionType _instMode = InstructionMode[lookup[instruction].operate].op;
    auto _addrMode = addrObj->get(addressMode).op;
    auto _instMode = instObj->get(instruction).op;

    uint8_t addrCycle = (e->*_addrMode)();
    uint8_t instCycle = (e->*_instMode)();

    return (cycles + (addrCycle & instCycle));
  }

  // Execute part instruction
  uint8_t Operation::execute(Executioner* e, Executioner::ExecutionType operation)
  {
    return (e->*operation)();
  }

};
