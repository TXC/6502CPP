#pragma once

#include "Graphics.hpp"

#include <string>
#include <streambuf>
#include <vector>
#include <map>
#include <fstream>

namespace NES
{
  enum rcode { FAIL = 0, OK = 1, NO_FILE = -1 };

  struct ResourceBuffer : public std::streambuf
  {
    ResourceBuffer(std::ifstream& ifs, uint32_t offset, uint32_t size);
    std::vector<char> vMemory;
  };

  class ResourcePack : public std::streambuf
  {
  public:
    ResourcePack();
    ~ResourcePack();
    bool AddFile(const std::string& sFile);
    bool LoadPack(const std::string& sFile, const std::string& sKey);
    bool SavePack(const std::string& sFile, const std::string& sKey);
    ResourceBuffer GetFileBuffer(const std::string& sFile);
    bool Loaded();
  private:
    struct sResourceFile { uint32_t nSize; uint32_t nOffset; };
    std::map<std::string, sResourceFile> mapFiles;
    std::ifstream baseFile;
    std::vector<char> scramble(const std::vector<char>& data, const std::string& key);
    std::string makeposix(const std::string& path);
  };


  class ImageLoader
  {
  public:
    ImageLoader() = default;
    virtual ~ImageLoader() = default;
    virtual rcode LoadImageResource(NES::Sprite* spr, const std::string& sImageFile, NES::ResourcePack* pack) = 0;
    virtual rcode SaveImageResource(NES::Sprite* spr, const std::string& sImageFile) = 0;
  };
};
