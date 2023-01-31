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

template <> struct fmt::formatter<CPU::Bus::MEMORYMAP> : fmt::ostream_formatter {};

template <> struct fmt::formatter<CPU::Executioner::tMode> : fmt::ostream_formatter {};
template <> struct fmt::formatter<CPU::Executioner::OperationType> : fmt::ostream_formatter {};

template <> struct fmt::formatter<CPU::Processor::DISASSEMBLY> : fmt::ostream_formatter {};
//template <> struct fmt::formatter<CPU::Processor::FLAGS6502> : fmt::ostream_formatter {};
template <> struct fmt::formatter<CPU::Processor::REGISTER> : fmt::ostream_formatter {};

/*
template <> struct fmt::formatter<CPU::Bus::MEMORYMAP> {
  constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
      return ctx.end();
  }

  template <typename FormatContext>
  auto format(CPU::Bus::MEMORYMAP const& input, FormatContext& ctx) -> decltype(ctx.out()) {
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

template <> struct fmt::formatter<CPU::Executioner::tMode> {
  constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
      return ctx.end();
  }

  template <typename FormatContext>
  auto format(CPU::Executioner::tMode const& input, FormatContext& ctx) -> decltype(ctx.out()) {
    return format_to(ctx.out(), "{}", input.name);
  }
};

template <> struct fmt::formatter<CPU::Executioner::OperationType> {
  constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
      return ctx.end();
  }

  template <typename FormatContext>
  auto format(CPU::Executioner::OperationType const& input, FormatContext& ctx) -> decltype(ctx.out()) {
    return format_to(
      ctx.out(),
      "{}:{} [{:d}]",
      input.operate.name, input.addrmode.name, input.cycles
    );
  }
};

template <> struct fmt::formatter<CPU::Processor::DISASSEMBLY> {
  constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
      return ctx.end();
  }

  template <typename FormatContext>
  auto format(CPU::Processor::DISASSEMBLY const& input, FormatContext& ctx) -> decltype(ctx.out()) {
    return format_to(
      ctx.out(),
      "{:02X} {:02X} {:02X} {:s} {:<30s}",
      input.OpCode, input.LowAddress, input.HighAddress,
      input.OpCodeString, input.DisassemblyOutput
    );
  }
};

template <> struct fmt::formatter<CPU::Processor::REGISTER> {
  constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
      return ctx.end();
  }

  template <typename FormatContext>
  auto format(CPU::Processor::REGISTER const& input, FormatContext& ctx) -> decltype(ctx.out()) {
    return format_to(
      ctx.out(),
      "PC:{:04X} {:<13s} A:{:02X} X:{:02X} Y:{:02X} {:s}{:s}{:s}{:s}{:s}{:s}{:s}{:s} STKP:{:02X}",
      input.PC, "XXX", input.AC, input.X, input.Y,
      (((input.SR & CPU::Processor::FLAGS6502::N) > 0) ? "N" : "."),
      (((input.SR & CPU::Processor::FLAGS6502::V) > 0) ? "V" : "."),
      (((input.SR & CPU::Processor::FLAGS6502::U) > 0) ? "U" : "."),
      (((input.SR & CPU::Processor::FLAGS6502::B) > 0) ? "B" : "."),
      (((input.SR & CPU::Processor::FLAGS6502::D) > 0) ? "D" : "."),
      (((input.SR & CPU::Processor::FLAGS6502::I) > 0) ? "I" : "."),
      (((input.SR & CPU::Processor::FLAGS6502::Z) > 0) ? "Z" : "."),
      (((input.SR & CPU::Processor::FLAGS6502::C) > 0) ? "C" : "."),
      input.SP
    );
  }
};
*/

/*
template <> struct fmt::formatter<CPU::Processor::FLAGS6502> {
  template <typename FormatContext>
  auto format(CPU::Processor::FLAGS6502 const& input, FormatContext& ctx) -> decltype(ctx.out()) {
    return format_to(
      ctx.out(),
      "{:s}{:s}{:s}{:s}{:s}{:s}{:s}{:s}",
      (((input.N & CPU::Processor::FLAGS6502::N) > 0) ? "N" : "."),
      (((input.V & CPU::Processor::FLAGS6502::V) > 0) ? "V" : "."),
      (((input.U & CPU::Processor::FLAGS6502::U) > 0) ? "U" : "."),
      (((input.B & CPU::Processor::FLAGS6502::B) > 0) ? "B" : "."),
      (((input.D & CPU::Processor::FLAGS6502::D) > 0) ? "D" : "."),
      (((input.I & CPU::Processor::FLAGS6502::I) > 0) ? "I" : "."),
      (((input.Z & CPU::Processor::FLAGS6502::Z) > 0) ? "Z" : "."),
      (((input.C & CPU::Processor::FLAGS6502::C) > 0) ? "C" : "."),
    );
  }
};
*/

