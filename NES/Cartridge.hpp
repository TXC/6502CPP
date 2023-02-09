#pragma once
#include <cstdint>
#include <string>
#include <fstream>
#include <vector>

#include "Mapper_000.hpp"
#include "Mapper_001.hpp"
#include "Mapper_002.hpp"
#include "Mapper_003.hpp"
#include "Mapper_004.hpp"
#include "Mapper_066.hpp"

namespace NES
{
  class Cartridge
  {
  public:
    Cartridge(const std::string& sFileName);
    ~Cartridge();
    
    
  public:
    bool ImageValid();
    
    
    
    
  private:
    bool bImageValid = false;
    MIRROR hw_mirror = HORIZONTAL;
    
    uint8_t nMapperID = 0;
    uint8_t nPRGBanks = 0;
    uint8_t nCHRBanks = 0;
    
    std::vector<uint8_t> vPRGMemory;
    std::vector<uint8_t> vCHRMemory;
    
    std::shared_ptr<Mapper> pMapper = nullptr;
    
  public:
    // Communication with Main Bus
    bool cpuRead(uint16_t addr, uint8_t &data);
    bool cpuWrite(uint16_t addr, uint8_t data);
    
    // Communication with PPU Bus
    bool ppuRead(uint16_t addr, uint8_t &data);
    bool ppuWrite(uint16_t addr, uint8_t data);
    
    // Permits system rest of mapper to know state
    void reset();
    
    // Get Mirror configuration
    MIRROR Mirror();
    
    std::shared_ptr<Mapper> GetMapper();
  };
};