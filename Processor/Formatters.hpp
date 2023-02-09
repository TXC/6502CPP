#pragma once

#include "MainBus.hpp"
#include "CPU.hpp"
#include "Executioner.hpp"

#if defined SPDLOG_FMT_EXTERNAL
#include <fmt/format.h>
#include <fmt/ostream.h>
#else
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ostr.h>
#endif
#include <iostream>

template <> struct fmt::formatter<Processor::Bus::MEMORYMAP> : fmt::ostream_formatter {};

template <> struct fmt::formatter<Processor::Executioner::tMode> : fmt::ostream_formatter {};
template <> struct fmt::formatter<Processor::Executioner::OperationType> : fmt::ostream_formatter {};

template <> struct fmt::formatter<Processor::CPU::DISASSEMBLY> : fmt::ostream_formatter {};
//template <> struct fmt::formatter<Processor::CPU::FLAGS6502> : fmt::ostream_formatter {};
template <> struct fmt::formatter<Processor::CPU::REGISTER> : fmt::ostream_formatter {};

/*
template <> struct fmt::formatter<Processor::Bus::MEMORYMAP> {
  constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
      return ctx.end();
  }

  template <typename FormatContext>
  auto format(Processor::Bus::MEMORYMAP const& input, FormatContext& ctx) -> decltype(ctx.out()) {
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

template <> struct fmt::formatter<Processor::Executioner::tMode> {
  constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
      return ctx.end();
  }

  template <typename FormatContext>
  auto format(Processor::Executioner::tMode const& input, FormatContext& ctx) -> decltype(ctx.out()) {
    return format_to(ctx.out(), "{}", input.name);
  }
};

template <> struct fmt::formatter<Processor::Executioner::OperationType> {
  constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
      return ctx.end();
  }

  template <typename FormatContext>
  auto format(Processor::Executioner::OperationType const& input, FormatContext& ctx) -> decltype(ctx.out()) {
    return format_to(
      ctx.out(),
      "{}:{} [{:d}]",
      input.operate.name, input.addrmode.name, input.cycles
    );
  }
};

template <> struct fmt::formatter<Processor::CPU::DISASSEMBLY> {
  constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
      return ctx.end();
  }

  template <typename FormatContext>
  auto format(Processor::CPU::DISASSEMBLY const& input, FormatContext& ctx) -> decltype(ctx.out()) {
    return format_to(
      ctx.out(),
      "{:02X} {:02X} {:02X} {:s} {:<30s}",
      input.OpCode, input.LowAddress, input.HighAddress,
      input.OpCodeString, input.DisassemblyOutput
    );
  }
};

template <> struct fmt::formatter<Processor::CPU::REGISTER> {
  constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
      return ctx.end();
  }

  template <typename FormatContext>
  auto format(Processor::CPU::REGISTER const& input, FormatContext& ctx) -> decltype(ctx.out()) {
    return format_to(
      ctx.out(),
      "PC:{:04X} {:<13s} A:{:02X} X:{:02X} Y:{:02X} {:s}{:s}{:s}{:s}{:s}{:s}{:s}{:s} STKP:{:02X}",
      input.PC, "XXX", input.AC, input.X, input.Y,
      (((input.SR & Processor::CPU::FLAGS6502::N) > 0) ? "N" : "."),
      (((input.SR & Processor::CPU::FLAGS6502::V) > 0) ? "V" : "."),
      (((input.SR & Processor::CPU::FLAGS6502::U) > 0) ? "U" : "."),
      (((input.SR & Processor::CPU::FLAGS6502::B) > 0) ? "B" : "."),
      (((input.SR & Processor::CPU::FLAGS6502::D) > 0) ? "D" : "."),
      (((input.SR & Processor::CPU::FLAGS6502::I) > 0) ? "I" : "."),
      (((input.SR & Processor::CPU::FLAGS6502::Z) > 0) ? "Z" : "."),
      (((input.SR & Processor::CPU::FLAGS6502::C) > 0) ? "C" : "."),
      input.SP
    );
  }
};
*/

/*
template <> struct fmt::formatter<Processor::CPU::FLAGS6502> {
  template <typename FormatContext>
  auto format(Processor::CPU::FLAGS6502 const& input, FormatContext& ctx) -> decltype(ctx.out()) {
    return format_to(
      ctx.out(),
      "{:s}{:s}{:s}{:s}{:s}{:s}{:s}{:s}",
      (((input.N & Processor::CPU::FLAGS6502::N) > 0) ? "N" : "."),
      (((input.V & Processor::CPU::FLAGS6502::V) > 0) ? "V" : "."),
      (((input.U & Processor::CPU::FLAGS6502::U) > 0) ? "U" : "."),
      (((input.B & Processor::CPU::FLAGS6502::B) > 0) ? "B" : "."),
      (((input.D & Processor::CPU::FLAGS6502::D) > 0) ? "D" : "."),
      (((input.I & Processor::CPU::FLAGS6502::I) > 0) ? "I" : "."),
      (((input.Z & Processor::CPU::FLAGS6502::Z) > 0) ? "Z" : "."),
      (((input.C & Processor::CPU::FLAGS6502::C) > 0) ? "C" : "."),
    );
  }
};
*/

