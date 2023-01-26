#pragma once

#include "Bus.hpp"
#include "Processor.hpp"
#include "Executioner.hpp"

#if defined SPDLOG_FMT_EXTERNAL
#include <fmt/format.h>
#include <fmt/ostream.h>
#else
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ostr.h>
#endif
#include <iostream>

using Proc = CPU::Processor;
using Exec = CPU::Executioner;


template <> struct fmt::formatter<CPU::Bus::MEMORYMAP> : ostream_formatter {};

template <> struct fmt::formatter<Exec::tMode> : ostream_formatter {};
template <> struct fmt::formatter<Exec::OperationType> : ostream_formatter {};

template <> struct fmt::formatter<Proc::DISASSEMBLY> : ostream_formatter {};
//template <> struct fmt::formatter<Proc::FLAGS6502> : ostream_formatter {};
template <> struct fmt::formatter<Proc::REGISTER> : ostream_formatter {};


/*
template<> struct fmt::formatter<Proc::DISASSEMBLY> : fmt::formatter<std::string> {
  template <typename FormatContext>
  auto format(Proc::DISASSEMBLY const& input, FormatContext& ctx) -> decltype(ctx.out()) {
    return format_to(
      ctx.out(),
      "{:02X} {:02X} {:02X} {:s} {:<30s}",
      obj.OpCode, obj.LowAddress, obj.HighAddress,
      obj.OpCodeString, obj.DisassemblyOutput
    );
  }
};
*/

/*
template<> struct fmt::formatter<Proc::MEMORYMAP> : fmt::formatter<std::string> {
  template <typename FormatContext>
  auto format(Proc::MEMORYMAP const& input, FormatContext& ctx) -> decltype(ctx.out()) {
    return format_to(
      ctx.out(),
      "{:04X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X}",
      input.Offset,
      input.Pos00, input.Pos01, input.Pos02, input.Pos03,
      input.Pos04, input.Pos05, input.Pos06, input.Pos07,
      input.Pos08, input.Pos09, input.Pos0A, input.Pos0B,
      input.Pos0C, input.Pos0D, input.Pos0E, input.Pos0F
    );
  }
};
*/

/*
template<> struct fmt::formatter<Proc::FLAGS6502> : fmt::formatter<std::string> {
  template <typename FormatContext>
  auto format(Proc::FLAGS6502 const& input, FormatContext& ctx) -> decltype(ctx.out()) {
    return format_to(
      ctx.out(),
      "{:s}{:s}{:s}{:s}{:s}{:s}{:s}{:s}",
      (((Proc::status & Proc::FLAGS6502::N) > 0) ? "N" : "."),
      (((Proc::status & Proc::FLAGS6502::V) > 0) ? "V" : "."),
      (((Proc::status & Proc::FLAGS6502::U) > 0) ? "U" : "."),
      (((Proc::status & Proc::FLAGS6502::B) > 0) ? "B" : "."),
      (((Proc::status & Proc::FLAGS6502::D) > 0) ? "D" : "."),
      (((Proc::status & Proc::FLAGS6502::I) > 0) ? "I" : "."),
      (((Proc::status & Proc::FLAGS6502::Z) > 0) ? "Z" : "."),
      (((Proc::status & Proc::FLAGS6502::C) > 0) ? "C" : "."),
    );
  }
};
*/

/*
template<> struct fmt::formatter<Proc::REGISTER> : fmt::formatter<std::string> {
  template <typename FormatContext>
  auto format(Proc::REGISTER const& input, FormatContext& ctx) -> decltype(ctx.out()) {
    return format_to(
      ctx.out(),
      "PC:{:04X} {:<13s} A:{:02X} X:{:02X} Y:{:02X} {:s}{:s}{:s}{:s}{:s}{:s}{:s}{:s} STKP:{:02X}",
      input.PC, "XXX", input.AC, input.X, input.Y,
      (((input.SR & Proc::FLAGS6502::N) > 0) ? "N" : "."),
      (((input.SR & Proc::FLAGS6502::V) > 0) ? "V" : "."),
      (((input.SR & Proc::FLAGS6502::U) > 0) ? "U" : "."),
      (((input.SR & Proc::FLAGS6502::B) > 0) ? "B" : "."),
      (((input.SR & Proc::FLAGS6502::D) > 0) ? "D" : "."),
      (((input.SR & Proc::FLAGS6502::I) > 0) ? "I" : "."),
      (((input.SR & Proc::FLAGS6502::Z) > 0) ? "Z" : "."),
      (((input.SR & Proc::FLAGS6502::C) > 0) ? "C" : "."),
      input.SP
    );
  }
};
*/

