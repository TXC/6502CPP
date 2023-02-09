#pragma once

#include "MainTest.hpp"
#include <Types.hpp>

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <filesystem>

namespace CPUTest
{
  using namespace CPU;
  class FunctionalProcessorTests
  {
  private:
    static std::fstream loadFile(std::string filename, std::ios::openmode openmode = std::ios::in)
    {
      std::fstream fp;

#if defined TESTDIR
      std::filesystem::path filepath = std::string(CPUSTRINGIZE(TESTDIR));
#else
      std::filesystem::path filepath = std::filesystem::current_path();
#endif

      filepath.append(filename);
      if (!std::filesystem::exists(filepath))
      {
        std::cout << "File not found\n"
          << "Filepath: " << filepath << "\n";
      }

      try {
        fp.open(filepath, openmode);
      } catch (const std::ios_base::failure& e) {
        std::cout << "Caught an ios_base::failure.\n"
                  << "Filepath: " << filepath.c_str() << "\n"
                  << "Filepath: " << filepath << "\n"
                  << "Filepath: " << filepath.string() << "\n"
                  << "Error code: " << e.code().value() 
                  << " (" << e.code().message() << ")\n"
                  << "Error category: " << e.code().category().name() << '\n';
      } catch (const std::exception& e) {
        std::cout << "EHH?! Caught an exception.\n"
                  << "Error code: " << e.what() << '\n';
      }

      return fp;
    }

  public:
    struct PROGRAMDATA
    {
      // Program
      uint8_t * program;
      //std::vector<uint8_t> program;
      // Program Size
      size_t size = 0x00;
    };

    struct TESTDATA
    {
      // 8-bit - Accumulator Register
      uint8_t  AC = 0x00;
      // 8-bit - X Register
      uint8_t  X = 0x00;
      // 8-bit - Y Register
      uint8_t  Y = 0x00;
      // 8-bit - Stack Pointer (points to location on bus)
      uint8_t  SP = 0x00;
      // 16-bit - Program Counter
      uint16_t PC = 0x0000;
      // 8-bit - Status Register
      uint8_t  SR = 0x00;
      // 32-bit - Cycles
      uint32_t CC = 0x00;
    };

    static PROGRAMDATA loadFunctionFile(std::string filename)
    {
      PROGRAMDATA data;

      try {
        std::fstream fp = loadFile(filename, std::ios::in | std::ios::binary);
        if (!fp)
        {
          std::cout << "Nope, cannot open file \"" << filename << "\" " << std::endl;
          exit(1);
        }

        if(fp.is_open())
        {
          std::streampos fsize = 0;
          fsize = fp.tellg();
          fp.seekg( 0, std::ios::end );
          fsize = fp.tellg() - fsize;
          fp.seekg( 0, std::ios::beg );

          std::cout << "Loaded file: \"" << filename << "\" - Size: " << fsize << " bytes.\n";

          std::vector<uint8_t> program;
          int i, j;
          while(fp.good())
          {
            char c[16];
            for(i = 0; i < 16 && fp.good(); i++)
            {
              fp.get(c[i]);
            }
            if (i < 16)
            {
              --i;
            }
            for(j = 0; j < i; j++)
            {
              program.push_back((uint8_t) c[(i - 1)]);
            }
          }
          fp.close();

          data = {
            .program = program.data(),
            .size    = program.size()
          };

          std::cout << "Finished file: \"" << filename << "\" - Read " << data.size << " bytes worth of data.\n";
        }

      } catch (const std::exception& e) {
        std::cout << "Caught an exception.\n"
                  << "Error code: " << e.what() << '\n';
      }
      return data;
    }

    static std::vector<TESTDATA> loadCycleTestResults(std::string filename, std::string delim = ",")
    {
      std::vector<TESTDATA> results;
      try {
        std::fstream fp = loadFile(filename, std::ios::in);
        if(fp.is_open())
        {
          std::streampos fsize = 0;
          fsize = fp.tellg();
          fp.seekg( 0, std::ios::end );
          fsize = fp.tellg() - fsize;
          fp.seekg( 0, std::ios::beg );

          std::cout << "Loaded file: \"" << filename << "\" - Size: " << fsize << " bytes.\n";

          std::string line;
          std::string word;
          uint16_t lineNumber = 0;
          while(std::getline(fp, line))
          {
            std::stringstream str(line);

            std::vector<std::string> values;
            while(std::getline(str, word, ',')) {
              values.push_back(word);
            }

            if ((uint16_t)std::stoul(values.at(0), nullptr, 10) % 2 != 0)
            {
              lineNumber++;
              continue;
            }

            if (values.size() == 9)
            {
              continue;
            }

            TESTDATA data = {
              .PC = (uint16_t) std::stoul(values.at(1), nullptr, 16),
              .AC = (uint8_t)  std::stoul(values.at(2), nullptr, 16),
              .X  = (uint8_t)  std::stoul(values.at(3), nullptr, 16),
              .Y  = (uint8_t)  std::stoul(values.at(4), nullptr, 16),
              .SR = (uint8_t)  std::stoul(values.at(5), nullptr, 16),
              .SP = (uint8_t)  std::stoul(values.at(6), nullptr, 16),
              .CC = (uint32_t) std::stoul(values.at(7), nullptr, 10)
            };

            results.push_back(data);
            lineNumber++;
          }
          std::cout << "Finished file: \"" << filename << "\" - Read " << results.size() << " rows of data.\n";
        }
        fp.close();
      } catch (const std::exception& e) {
        std::cout << "Caught an exception.\n"
                  << "Error code: " << e.what() << '\n';
      }
      return results;
    }
  };
};
