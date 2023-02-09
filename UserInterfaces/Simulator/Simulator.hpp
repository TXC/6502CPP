#pragma once

#include <NES/NESBus.hpp>
#include <memory>

class Simulator
{
private:
  std::unique_ptr<NES::NESBus> bus;

public:
  Simulator() {
    bus = std::make_unique<NES::NESBus>();
  };

  ~Simulator() {
  };

  void setApplication();

  void renderApplication();

  void ProgramController();

  void RegistersWindow();

  void MemoryTable(uint16_t offsetStart, uint16_t offsetStop);

  void Frame();
};
