#pragma once

#include "math.hpp"
#include <memory>
#include <raylib-cpp.hpp>
#include <utility>

namespace softrays {

class Texture {
  public:
  Texture(Colour colour) : Tint(colour) { }
  Texture() = default;
  Texture(const Texture&) = default;
  Texture(Texture&&) = default;
  Texture& operator=(const Texture&) = default;
  Texture& operator=(Texture&&) = default;
  virtual ~Texture() = default;

  [[nodiscard]] virtual Colour Value([[maybe_unused]] double u, [[maybe_unused]] double v, [[maybe_unused]] const Point3& loc) const
  {
    return Tint;
  }

  Colour Tint{1.0, 1.0, 1.0};
};

// Not strictly needed, as I've added Tint to the base class
class SolidTexture : public Texture {
  public:
  SolidTexture(Colour colour) : Texture(colour) { }
  [[nodiscard]] Colour Value([[maybe_unused]] double u, [[maybe_unused]] double v, [[maybe_unused]] const Point3& loc) const override
  {
    return Tint;
  }
};

class CheckeredTexture : public Texture {
  public:
  CheckeredTexture(double scale, std::shared_ptr<Texture> evenTexture, std::shared_ptr<Texture> oddTexture)
      : inv_scale(1.0 / scale), even(std::move(evenTexture)), odd(std::move(oddTexture)) { }

  CheckeredTexture(double scale, const Colour& col1, const Colour& col2)
      : CheckeredTexture(scale, std::make_shared<SolidTexture>(col1), std::make_shared<SolidTexture>(col2)) { }

  CheckeredTexture(Colour colour) : Texture(colour) { }
  [[nodiscard]] Colour Value(double u, double v, const Point3& loc) const override
  {
    auto xInteger = int(std::floor(inv_scale * loc.x));
    auto yInteger = int(std::floor(inv_scale * loc.y));
    auto zInteger = int(std::floor(inv_scale * loc.z));

    bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

    return (isEven ? even->Value(u, v, loc) : odd->Value(u, v, loc)) * Tint;
  }

  private:
  double inv_scale{};
  std::shared_ptr<Texture> even;
  std::shared_ptr<Texture> odd;
};

class ImageTexture : public Texture {
  public:
  ImageTexture(Colour tint, const char* filename) : Texture(tint), image(filename) { image.Format(PIXELFORMAT_UNCOMPRESSED_R32G32B32); }
  ImageTexture(const char* filename) : image(filename) { image.Format(PIXELFORMAT_UNCOMPRESSED_R32G32B32); }

  [[nodiscard]] Colour Value(double u, double v, [[maybe_unused]] const Point3& loc) const override
  {
    // If we have no texture data, then return solid cyan as a debugging aid.
    if (!image.IsReady())
      return Colour{0, 1, 1};

    // Clamp input texture coordinates to [0,1] x [1,0]
    u = Interval(0, 1).Clamp(u);
    v = 1.0 - Interval(0, 1).Clamp(v);  // Flip V to image coordinates

    auto i = int(u * image.width);
    auto j = int(v * image.height);
    const auto& pixel = static_cast<float*>(image.data) + (static_cast<std::ptrdiff_t>(i * 3)) + (static_cast<std::ptrdiff_t>(j * 3));

    auto color_scale = 1.0F;  // / 255.0F;
    return Colour{double(color_scale * pixel[0]), double(color_scale * pixel[1]), double(color_scale * pixel[2])};
  }

  private:
  raylib::Image image;
};
}
