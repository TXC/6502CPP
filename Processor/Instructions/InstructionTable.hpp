#pragma once

#include <map>
#include "../Singleton.hpp"
#include "../Executioner.hpp"

namespace CPU
{
    class Executioner;
    namespace Instructions
    {
        class InstructionTable final : public Singleton<InstructionTable>
        {
            //private:
            //    Operation *operation = nullptr;

            public:
                // This structure and the following vector are used to compile and store
                // the opcode translation table. The 6502 can effectively have 256
                // different instructions. Each of these are stored in a table in numerical
                // order so they can be looked up easily, with no decoding required.
                // Each table entry holds:
                //  Pneumonic : A textual representation of the instruction (used for disassembly)
                //  Opcode Function: A function pointer to the implementation of the opcode
                //  Opcode Address Mode : A function pointer to the implementation of the 
                //                        addressing mechanism used by the instruction
                //  Cycle Count : An integer that represents the base number of clock cycles the
                //                CPU requires to perform the instruction
                struct OperationType
                {
                    uint8_t operate  = 0x00;
                    uint8_t addrmode = 0x00;
                    uint8_t cycles   = 0;
                };
                struct ExpandedOperationType
                {
                    Executioner::tMode operate;
                    Executioner::tMode addrmode;
                    uint8_t            cycles = 0;
                };

                //void connectOperation(Operation *n) { operation = n; }

            private:
                std::map<uint8_t, OperationType> lookup = {};

            protected:
                InstructionTable();

            public:
                uint8_t exists(uint8_t opcode);
                OperationType get(uint8_t opcode);
                ExpandedOperationType getExpanded(uint8_t opcode);
        };
    };
};
