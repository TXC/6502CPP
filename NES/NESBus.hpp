#pragma once

#include "APU.hpp"
#include "PPU.hpp"
#include "Cartridge.hpp"

#include <Processor/Bus.hpp>
#include <Processor/CPU.hpp>

#include <array>
#include <map>
#include <cstdint>
#if defined SPDLOG_FMT_EXTERNAL
#include <fmt/format.h>
#include <fmt/ostream.h>
#else
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ostr.h>
#endif


namespace NES
{
  class NESBus : public Processor::Bus
  {
  public:
    NESBus();
    ~NESBus();

  public: // Devices on bus
    // The 6502 derived processor
    Processor::CPU cpu;
    // The 2C02 Picture Processing Unit
    PPU ppu;
    // The "2A03" Audio Processing Unit
    APU apu;
    // The Cartridge or "GamePak"
    std::shared_ptr<Cartridge> cart;
    // The 6502 RAM, 2KB
    std::array<uint8_t, 2048> ram;
    // Controllers
    uint8_t controller[2];

    // Synchronisation with system Audio
  public:
    void SetSampleFrequency(uint32_t sample_rate);
    double dAudioSample = 0.0;
    
  private:
    double dAudioTime = 0.0;
    double dAudioGlobalTime = 0.0;
    double dAudioTimePerNESClock = 0.0;
    double dAudioTimePerSystemSample = 0.0f;
    
    
  public: // Main Bus Read & Write
    void    cpuWrite(uint16_t addr, uint8_t data) override;
    uint8_t cpuRead(uint16_t addr, bool bReadOnly = false) override;
    
  private:
    // A count of how many clocks have passed
    uint32_t nSystemClockCounter = 0;
    // Internal cache of controller state
    uint8_t controller_state[2];
    
  private:
    // A simple form of Direct Memory Access is used to swiftly
    // transfer data from CPU bus memory into the OAM memory. It would
    // take too long to sensibly do this manually using a CPU loop, so
    // the program prepares a page of memory with the sprite info required
    // for the next frame and initiates a DMA transfer. This suspends the
    // CPU momentarily while the PPU gets sent data at PPU clock speeds.
    // Note here, that dma_page and dma_addr form a 16-bit address in
    // the CPU bus address space
    uint8_t dma_page = 0x00;
    uint8_t dma_addr = 0x00;
    uint8_t dma_data = 0x00;
    
    // DMA transfers need to be timed accurately. In principle it takes
    // 512 cycles to read and write the 256 bytes of the OAM memory, a
    // read followed by a write. However, the CPU needs to be on an "even"
    // clock cycle, so a dummy cycle of idleness may be required
    bool dma_dummy = true;
    
    // Finally a flag to indicate that a DMA transfer is happening
    bool dma_transfer = false;

//  public:
//    // A count of how many clocks have passed
//    uint32_t nSystemClockCounter = 0;

  public: // System Interface
    // Connects a cartridge object to the internal buses
    void insertCartridge(const std::shared_ptr<Cartridge>& cartridge);
    // Resets the system
    void reset() override;
    // Clocks the system - a single whole systme tick
    bool clock() override;
    // Check if the system is complete
    bool complete() override;
  };
};
