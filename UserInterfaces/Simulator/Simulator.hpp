#pragma once

#include <Bus.hpp>
#include <memory>

using namespace CPU;

class Bus;
class Simulator
{
private:
  std::unique_ptr<CPU::Bus> bus;

public:
  Simulator() {
    bus = std::make_unique<CPU::Bus>();
  };
  ~Simulator() {
  };

  void setApplication();

  void renderApplication();

  void ProgramController();

  void RegistersWindow();

  void MemoryTable(uint16_t offsetStart, uint16_t offsetStop);
};


