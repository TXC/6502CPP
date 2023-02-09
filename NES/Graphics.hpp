#pragma once

#include <memory>
#include <vector>

namespace NES
{

  static constexpr uint8_t  nDefaultAlpha = 0xFF;
  static constexpr uint32_t nDefaultPixel = (nDefaultAlpha << 24);

  struct Pixel
  {
    union
    {
      uint32_t n = nDefaultPixel;
      struct { uint8_t r; uint8_t g; uint8_t b; uint8_t a; };
    };
    
    enum Mode { NORMAL, MASK, ALPHA, CUSTOM };
    
    Pixel();
    Pixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = nDefaultAlpha);
    Pixel(uint32_t p);
    Pixel& operator = (const Pixel& v) = default;
    bool   operator ==(const Pixel& p) const;
    bool   operator !=(const Pixel& p) const;
    Pixel  operator * (const float i) const;
    Pixel  operator / (const float i) const;
    Pixel& operator *=(const float i);
    Pixel& operator /=(const float i);
    Pixel  operator + (const Pixel& p) const;
    Pixel  operator - (const Pixel& p) const;
    Pixel& operator +=(const Pixel& p);
    Pixel& operator -=(const Pixel& p);
    Pixel  operator * (const Pixel& p) const;
    Pixel& operator *=(const Pixel& p);
    Pixel  inv() const;
  };

  class Sprite
  {
  public:
    Sprite();
    Sprite(int32_t w, int32_t h);
    Sprite(const Sprite&) = delete;
    ~Sprite();
    
  public:
    int32_t width = 0;
    int32_t height = 0;
    enum Mode { NORMAL, PERIODIC, CLAMP };
    enum Flip { NONE = 0, HORIZ = 1, VERT = 2 };
    
  public:
    void SetSampleMode(Sprite::Mode mode = Sprite::Mode::NORMAL);
    Pixel GetPixel(int32_t x, int32_t y) const;
    bool  SetPixel(int32_t x, int32_t y, Pixel p);
    Pixel Sample(float x, float y) const;
    Pixel SampleBL(float u, float v) const;
    Pixel* GetData();
    Sprite* Duplicate();
    std::vector<Pixel> pColData;
    Mode modeSample = Mode::NORMAL;
  };
};
