#include <Graphics.hpp>

#include <algorithm>
#include <cmath>

namespace NES
{
  Sprite::Sprite()
  { width = 0; height = 0; }
  
  Sprite::Sprite(int32_t w, int32_t h)
  {
    width = w;    height = h;
    pColData.resize(width * height);
    pColData.resize(width * height, nDefaultPixel);
  }
  
  Sprite::~Sprite()
  { pColData.clear();  }
  
  void Sprite::SetSampleMode(NES::Sprite::Mode mode)
  { modeSample = mode; }
  
  Pixel Sprite::GetPixel(int32_t x, int32_t y) const
  {
    if (modeSample == NES::Sprite::Mode::NORMAL)
    {
      if (x >= 0 && x < width && y >= 0 && y < height)
        return pColData[y * width + x];
      else
        return Pixel(0, 0, 0, 0);
    }
    else
    {
      if (modeSample == NES::Sprite::Mode::PERIODIC)
        return pColData[abs(y % height) * width + abs(x % width)];
      else
        return pColData[std::max(0, std::min(y, height-1)) * width + std::max(0, std::min(x, width-1))];
    }
  }
  
  bool Sprite::SetPixel(int32_t x, int32_t y, Pixel p)
  {
    if (x >= 0 && x < width && y >= 0 && y < height)
    {
      pColData[y * width + x] = p;
      return true;
    }
    else
      return false;
  }
  
  Pixel Sprite::Sample(float x, float y) const
  {
    int32_t sx = std::min((int32_t)((x * (float)width)), width - 1);
    int32_t sy = std::min((int32_t)((y * (float)height)), height - 1);
    return GetPixel(sx, sy);
  }

  Pixel Sprite::SampleBL(float u, float v) const
  {
    u = u * width - 0.5f;
    v = v * height - 0.5f;
    int x = (int)std::floor(u); // cast to int rounds toward zero, not downward
    int y = (int)std::floor(v); // Thanks @joshinils
    float u_ratio = u - x;
    float v_ratio = v - y;
    float u_opposite = 1 - u_ratio;
    float v_opposite = 1 - v_ratio;
    
    Pixel p1 = GetPixel(std::max(x, 0), std::max(y, 0));
    Pixel p2 = GetPixel(std::min(x + 1, (int)width - 1), std::max(y, 0));
    Pixel p3 = GetPixel(std::max(x, 0), std::min(y + 1, (int)height - 1));
    Pixel p4 = GetPixel(std::min(x + 1, (int)width - 1), std::min(y + 1, (int)height - 1));
    
    return Pixel(
                      (uint8_t)((p1.r * u_opposite + p2.r * u_ratio) * v_opposite + (p3.r * u_opposite + p4.r * u_ratio) * v_ratio),
                      (uint8_t)((p1.g * u_opposite + p2.g * u_ratio) * v_opposite + (p3.g * u_opposite + p4.g * u_ratio) * v_ratio),
                      (uint8_t)((p1.b * u_opposite + p2.b * u_ratio) * v_opposite + (p3.b * u_opposite + p4.b * u_ratio) * v_ratio));
  }
  
  Pixel* Sprite::GetData()
  { return pColData.data(); }
  
  
  Sprite* Sprite::Duplicate()
  {
    Sprite* spr = new NES::Sprite(width, height);
    std::memcpy(spr->GetData(), GetData(), width * height * sizeof(NES::Pixel));
    spr->modeSample = modeSample;
    return spr;
  }
/*
  olc::vi2d olc::Sprite::Size() const
  {
    return { width, height };
  }
*/
};
