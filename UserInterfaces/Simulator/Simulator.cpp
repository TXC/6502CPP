#include "Simulator.hpp"

#include <Bus.hpp>
#include <Processor.hpp>
#include <Formatters.hpp>

#include <imgui.h>
#include <cstdint>
#include <iostream>
#include <map>

#if defined SPDLOG_FMT_EXTERNAL
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/ostream.h>
#else
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ranges.h>
#include <spdlog/fmt/ostr.h>
#endif

//                                X    Y
//ImGui::SetNextWindowPos(ImVec2(200, 10), ImGuiCond_FirstUseEver);
//                                W    H
//ImGui::SetNextWindowSize(ImVec2(300, 120), ImGuiCond_FirstUseEver);

void Simulator::setApplication()
{
  uint8_t program[] = {
    0xA2, 0x0A, 0x8E, 0x00, 0x00, 0xA2, 0x03, 0x8E, 0x01, 0x00, 0xAC, 0x00, 0x00, 0xA9,
    0x00, 0x18, 0x6D, 0x01, 0x00, 0x88, 0xD0, 0xFA, 0x8D, 0x02, 0x00, 0xEA, 0xEA, 0xEA
  };
  size_t n = sizeof(program) / sizeof(program[0]);
  bus->cpu->loadProgram(0x8000, program, n, 0x8000);
}

void Simulator::renderApplication() {
  ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(300, 140), ImGuiCond_FirstUseEver);
  ProgramController();

  ImGui::SetNextWindowPos(ImVec2(320, 10), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(250, 140), ImGuiCond_FirstUseEver);
  RegistersWindow();

  ImGui::SetNextWindowPos(ImVec2(425, 350), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(410, 180), ImGuiCond_FirstUseEver);
  MemoryTable(0x8000, 0x807F);

  ImGui::SetNextWindowPos(ImVec2(10, 350), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(415, 325), ImGuiCond_FirstUseEver);
  MemoryTable(0x100, 0x1FF);
}

void Simulator::ProgramController()
{
  ImGui::Begin("Program Control"); 

  static bool run_button_disable = bus->cpu->getJammed() || (bus->cpu->getOpCode() == 0 && bus->cpu->getOperationCycleCount() > 0);
  static bool reset_button_disable = bus->cpu->getJammed();

  ImGui::BeginDisabled(run_button_disable);

  if (ImGui::Button("Run cycle")) {
    bus->cpu->tick();
  }

  ImGui::EndDisabled();

  ImGui::SameLine();

  ImGui::BeginDisabled(reset_button_disable);

  if (ImGui::Button("Reset")) {
    bus->cpu->reset();
  }

  ImGui::EndDisabled();

  ImGui::SameLine();

  if (ImGui::Button("Hard reset")) {
    bus->cpu->reset();
    bus.reset();
  }

  //ImGui::SliderInt("CPU Speed", &bus->cpu->cpuspeed, 1, 10, "%d");

  if (ImGui::IsKeyDown(ImGuiKey_Space) && run_button_disable == false) {
    bus->cpu->tick();
  }

  ImGui::End();
}

void Simulator::RegistersWindow()
{
  ImGui::Begin("Registers");

  //uint8_t r = bus->cpu->getRegister(CPU::Processor::REGISTER6502::SR);
  //fmt::print("REGISTER: {}", r);

  ImVec4 flagActive     = ImVec4(0.0f, 1.0f, 0.6f, 1.0f);
  ImVec4 flagInactive   = ImVec4(1.0f, 0.1f, 0.3f, 1.0f);
/*
  std::string buf = fmt::format(
    "AC: {0:02X}   {0:03d} X  : {1:02X}   {1:03d}\nY  : {2:02X}   {2:03d} SP : {3:02X}   {3:03d}\nPC : {4:04X} ({4:05d})\nOpCode : {5:02X}",
    bus->cpu->getRegisterAC(), bus->cpu->getRegisterX(),
    bus->cpu->getRegisterY(), bus->cpu->getRegisterSP(),
    bus->cpu->getProgramCounter(), bus->cpu->getOpCode()
  );
*/

  ImGui::Text("AC : %1$02X   (%1$03d)", bus->cpu->getRegisterAC());
  ImGui::SameLine();
  ImGui::Text(" ");
  ImGui::SameLine();
  ImGui::Text("X  : %1$02X   (%1$03d)", bus->cpu->getRegisterX());

  ImGui::Text("Y  : %1$02X   (%1$03d)", bus->cpu->getRegisterY());
  ImGui::SameLine();
  ImGui::Text(" ");
  ImGui::SameLine();
  ImGui::Text("SP : %1$02X   (%1$03d)", bus->cpu->getRegisterSP());
  
  ImGui::Text("PC : %1$04X (%1$05d)", bus->cpu->getProgramCounter());
  
  ImGui::Text("Total Cycles : %d", bus->cpu->getOperationCycleCount());

  ImGui::Text("OpCode : %1$02X  %2$s / %3$s",
    bus->cpu->getOpCode(),
    bus->cpu->executioner.getInstructionName(bus->cpu->getOpCode()).c_str(),
    bus->cpu->executioner.getAddressModeName(bus->cpu->getOpCode()).c_str()
  );

  ImGui::TextColored(bus->cpu->getFlag(bus->cpu->N) ? flagActive : flagInactive, "N");
  ImGui::SameLine();

  ImGui::TextColored(bus->cpu->getFlag(bus->cpu->V) ? flagActive : flagInactive, "V");
  ImGui::SameLine();

  ImGui::TextColored(bus->cpu->getFlag(bus->cpu->U) ? flagActive : flagInactive, "U");
  ImGui::SameLine();

  ImGui::TextColored(bus->cpu->getFlag(bus->cpu->B) ? flagActive : flagInactive, "B");
  ImGui::SameLine();

  ImGui::TextColored(bus->cpu->getFlag(bus->cpu->D) ? flagActive : flagInactive, "D");
  ImGui::SameLine();

  ImGui::TextColored(bus->cpu->getFlag(bus->cpu->I) ? flagActive : flagInactive, "I");
  ImGui::SameLine();

  ImGui::TextColored(bus->cpu->getFlag(bus->cpu->Z) ? flagActive : flagInactive, "Z");
  ImGui::SameLine();

  ImGui::TextColored(bus->cpu->getFlag(bus->cpu->C) ? flagActive : flagInactive, "C");

  ImGui::End();
}

void Simulator::MemoryTable(uint16_t offsetStart, uint16_t offsetStop)
{
  ImGui::Begin(fmt::format("Memory Map [${:04X}- ${:04X}]", offsetStart, offsetStop).c_str());

  ImGuiTableFlags memoryFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_Hideable;

  ImVec4 flagActive     = ImVec4(0.0f, 1.0f, 0.6f, 1.0f);

  if (ImGui::BeginTable("memorymap", 17, memoryFlags))
  {
    ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
    ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("00", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("01", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("02", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("03", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("04", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("05", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("06", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("07", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("08", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("09", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("0A", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("0B", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("0C", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("0D", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("0E", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("0F", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableHeadersRow();

    std::map<uint16_t, CPU::Bus::MEMORYMAP> memorymap = bus->memoryDump(offsetStart, offsetStop);
    for (uint16_t n = 0; n < (memorymap.size() - 1) ; ++n)
    {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();

      ImGui::Text("$%04X", memorymap[n].Offset);
      ImGui::TableNextColumn();

      ImGui::Text("%02X", memorymap[n].Pos00);
      ImGui::TableNextColumn();

      ImGui::Text("%02X", memorymap[n].Pos01);
      ImGui::TableNextColumn();

      ImGui::Text("%02X", memorymap[n].Pos02);
      ImGui::TableNextColumn();

      ImGui::Text("%02X", memorymap[n].Pos03);
      ImGui::TableNextColumn();

      ImGui::Text("%02X", memorymap[n].Pos04);
      ImGui::TableNextColumn();

      ImGui::Text("%02X", memorymap[n].Pos05);
      ImGui::TableNextColumn();

      ImGui::Text("%02X", memorymap[n].Pos06);
      ImGui::TableNextColumn();

      ImGui::Text("%02X", memorymap[n].Pos07);
      ImGui::TableNextColumn();

      ImGui::Text("%02X", memorymap[n].Pos08);
      ImGui::TableNextColumn();

      ImGui::Text("%02X", memorymap[n].Pos09);
      ImGui::TableNextColumn();

      ImGui::Text("%02X", memorymap[n].Pos0A);
      ImGui::TableNextColumn();

      ImGui::Text("%02X", memorymap[n].Pos0B);
      ImGui::TableNextColumn();

      ImGui::Text("%02X", memorymap[n].Pos0C);
      ImGui::TableNextColumn();

      ImGui::Text("%02X", memorymap[n].Pos0D);
      ImGui::TableNextColumn();

      ImGui::Text("%02X", memorymap[n].Pos0E);
      ImGui::TableNextColumn();

      ImGui::Text("%02X", memorymap[n].Pos0F);


      for(uint8_t col = 0; col <= 0xF; ++col) {
        if (bus->cpu->getProgramCounter() == (uint16_t)(memorymap[n].Offset + col)) {
          ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
          ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color, (col + 1));
        }
      }
    }
    ImGui::EndTable();
  }

  ImGui::End();
}





