#pragma once

#include "math.hpp"
#include <memory>
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
  CheckeredTexture(double scale, std::shared_ptr<Texture> even, std::shared_ptr<Texture> odd)
      : inv_scale(1.0 / scale), even(std::move(even)), odd(std::move(odd)) { }

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
}
