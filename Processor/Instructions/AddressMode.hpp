#pragma once

#include <map>
//#include "../Types.hpp"
#include "../Singleton.hpp"
#include "../Operation.hpp"
//#include "../Executioner.hpp"

namespace CPU
{
  class Executioner;
  namespace Instructions
  {
    class AddressMode final : public Singleton<AddressMode>
    {
    private:
      std::map<uint8_t, Executioner::tMode> lookup = {};
      //Operation *operation = nullptr;

    public:
      struct AddressingModes
      {
        static const uint8_t Accumulator = 0;
        static const uint8_t Implied = 1;
        static const uint8_t Immediate = 2;
        static const uint8_t Relative = 3;
        static const uint8_t Absolute = 4;
        static const uint8_t AbsoluteX = 5;
        static const uint8_t AbsoluteY = 6;
        static const uint8_t ZeroPage = 7;
        static const uint8_t ZeroPageX = 8;
        static const uint8_t ZeroPageY = 9;
        static const uint8_t Indirect = 10;
        static const uint8_t IndirectX = 11;
        static const uint8_t IndirectY = 12;
      };
      //void connectOperation(Operation *n) { operation = n; }

    protected:
      AddressMode();

    public:
      uint8_t exists(uint8_t opcode);
      Executioner::tMode get(uint8_t opcode);
    };
  };
};
