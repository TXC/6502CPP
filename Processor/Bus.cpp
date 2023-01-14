#include <iostream>
#include "Bus.hpp"
#include "Log.hpp"

namespace CPU
{

Bus::Bus()
{
    // Connect CPU to communication bus
    cpu.ConnectBus(this);

    // Clear RAM contents, just in case :P
    reset();
}


Bus::~Bus()
{
}

void Bus::reset()
{
    for (auto &i : ram)
    {
        i = 0x00;
    }
}

void Bus::write(uint16_t addr, uint8_t data)
{
    if (addr >= 0x0000 && addr <= 0xFFFF)
    {
        ram[addr] = data;
    }
}

uint8_t Bus::read(uint16_t addr, bool bReadOnly)
{
    if (addr >= 0x0000 && addr <= 0xFFFF)
    {
        return ram[addr];
    }

    return 0x00;
}

void Bus::dump(uint16_t offset)
{
#ifdef LOGMODE
    fprintf(CPU::logfile, "Actual ADDR: $%04X\n", offset);
    uint16_t offsetStart = offset & 0xFFF0;
    uint16_t offsetStop = offset | 0x000F;
    dump(offsetStart, offsetStop);
#endif
}

void Bus::dump(uint16_t offsetStart, uint16_t offsetStop)
{
#ifdef LOGMODE
    std::string log = dumpRaw(offsetStart, offsetStop);
    fprintf(CPU::logfile, "%s", log.c_str());
    fflush(CPU::logfile);
#endif
}

std::string Bus::dumpRaw(uint16_t offsetStart, uint16_t offsetStop)
{
    std::string log = string_format("MEMORY LOG FOR: $%04X - $%04X \n$%04X:", offsetStart, offsetStop, offsetStart);

    uint16_t multiplier = 0;
    for(uint16_t i = offsetStart; i <= offsetStop; i++)
    {
        if (i % 16 == 0 && i != offsetStart) {
            multiplier++;
            if (i != offsetStop)
            {
                log += string_format("\n$%04X:", offsetStart + (multiplier * 0x0010));
            }
        }

        log += string_format(" %02X", read(i, true));
        if (i == offsetStop)
        {
            log += "\n";
        }
    }

    return log;
}
}