#pragma once

#include "Formatters.hpp"
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/pattern_formatter.h>

namespace CPU
{
  static std::shared_ptr<spdlog::logger> logger = nullptr;
  class Logger
  {
  public:
    Logger();
    ~Logger();

  private:
    static void createLogger();

  public:
    static std::shared_ptr<spdlog::logger> log();
    static std::shared_ptr<spdlog::logger> log(std::string text);
  };

  class custom_flag : public spdlog::custom_flag_formatter
  {
  public:
    explicit custom_flag(std::string txt)
        : some_txt{std::move(txt)}
    {}

    void format(const spdlog::details::log_msg &, const std::tm &, spdlog::memory_buf_t &dest) override
    {
        std::string some_txt = "custom-flag";
        some_txt = std::string(padinfo_.width_, ' ') + some_txt;
        dest.append(some_txt.data(), some_txt.data() + some_txt.size());
    }

    spdlog::details::padding_info get_padding_info()
    {
        return padinfo_;
    }

    std::string some_txt;

    std::unique_ptr<custom_flag_formatter> clone() const override
    {
        return spdlog::details::make_unique<custom_flag>(some_txt);
    }
  };
};
