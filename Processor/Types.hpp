#pragma once

//#include "Executioner.hpp"

#include <memory>
#include <string>
#include <map>
#include <iostream>
#ifdef SPDLOG_FMT_EXTERNAL
#include <fmt/format.h>
//#include <fmt/ostream.h>
#else
#include <spdlog/fmt/fmt.h>
//#include <spdlog/fmt/ostr.h>
#endif
#include <stdexcept>
#include <vector>

#ifdef DEBUG
#include <execinfo.h>
#include <dlfcn.h>
#include <cxxabi.h>
#include <sstream>
#endif

namespace CPU
{
  class Executioner;

  // A convenient utility to search map-based types
  // after a specific value (needle).
  template <typename T>
  bool in_array(const T& needle, const std::vector<T>& haystack)
  {
    size_t max = haystack.size();
    if (max == 0)
    {
      return false;
    }

    for (auto i : haystack)
    {
      if (i == needle)
      {
        return true;
      }
    }
    return false;
  }

  template<typename ... Args>
  std::string string_format( const std::string& format, Args ... args )
  {
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
  }

  // A convenient utility to convert variables into
  // hex strings because "modern C++"'s method with 
  // streams is atrocious
  template <typename T = std::string>
  T hex(uint32_t n, uint8_t d)
  {
    std::string s(d, '0');
    for (int i = d - 1; i >= 0; i--, n >>= 4)
    {
      s[i] = "0123456789ABCDEF"[n & 0xF];
    }
    return s;
  };

  template <typename T = std::string>
  std::string Backtrace(int skip = 1)
  {
    void *callstack[128];
    const int nMaxFrames = sizeof(callstack) / sizeof(callstack[0]);
    char buf[1024];
    int nFrames = backtrace(callstack, nMaxFrames);

    std::ostringstream trace_buf;
    for (int i = skip; i < nFrames; i++) {
      Dl_info info;
      if (dladdr(callstack[i], &info)) {
        char *demangled = NULL;
        int status;
        demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
        snprintf(buf, sizeof(buf), "%-3d %*p %s + %zd\n",
            i, (int)(2 + (sizeof(void*) * 2)), callstack[i],
            status == 0 ? demangled : info.dli_sname,
            (char *)callstack[i] - (char *)info.dli_saddr);
        free(demangled);
      } else {
        snprintf(buf, sizeof(buf), "%-3d %*p\n",
            i, (int)(2 + (sizeof(void*) * 2)), callstack[i]);
      }
      trace_buf << buf;
    }
    if (nFrames == nMaxFrames)
      trace_buf << "  [truncated]\n";
    return trace_buf.str();
  }

#ifdef DEBUG
  // this function will re-throw the current exception, nested inside a
  // new one. If the std::current_exception is derived from logic_error, 
  // this function will throw a logic_error. Otherwise it will throw a
  // runtime_error
  // The message of the exception will be composed of the arguments
  // context and the variadic arguments args... which may be empty.
  // The current exception will be nested inside the new one
  // @pre context and args... must support ostream operator <<
  template<class Context, class...Args>
  void rethrow(Context&& context, Args&&... args)
  {
    // build an error message
    std::ostringstream ss;
    ss << context;
    auto sep = " : ";
    using expand = int[];
    void (expand{ 0, ((ss << sep << args), sep = ", ", 0)... });
    // figure out what kind of exception is active
    try {
        std::rethrow_exception(std::current_exception());
    }
    catch(const std::invalid_argument& e) {
        std::throw_with_nested(std::invalid_argument(ss.str()));
    }
    catch(const std::logic_error& e) {
        std::throw_with_nested(std::logic_error(ss.str()));
    }
    // etc - default to a runtime_error 
    catch(...) {
        std::throw_with_nested(std::runtime_error(ss.str()));
    }
  }

  // unwrap nested exceptions, printing each nested exception to 
  // std::cerr
  template <typename T = std::exception>
  void print_exception (const T& e, std::size_t depth = 0) {
    std::cerr << "exception: " << std::string(depth, ' ') << e.what() << '\n';
    try {
      std::rethrow_if_nested(e);
    } catch (const T& nested) {
      print_exception(nested, depth + 1);
    } catch (const std::exception& nested) {
      print_exception(nested, depth + 1);
    }
  }
#endif
};
