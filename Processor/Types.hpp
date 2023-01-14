#pragma once

#include <memory>
#include <string>
#include <map>
#include <stdexcept>
//#include <cstdio>

namespace CPU
{
  // A convenient utility to search map-based types
  // after a specific value (needle).
  template <typename T>
  bool in_array(const T& needle, const std::vector<T>& haystack)
  {
    int max = haystack.size();
    if (max == 0)
    {
      return false;
    }

    for (int i = 0; i < max; i++)
    {
      if (haystack[i] == needle)
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


/*
  // Utility to format std::string with printf syntax
  template<typename ... Args>
  std::string string_format(const std::string& format, Args ... args)
  {
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    if (size_s <= 0)
    {
      throw std::runtime_error("Error during formatting.");
    }
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
  }
*/


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

};
