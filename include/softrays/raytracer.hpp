#pragma once

// #include <cstdint>
// #include <span>

#include "utility.hpp"

class RayTracer {
  public:
  struct Ray {
    Point3 Origin;
    Vec3 Direction;

    [[nodiscard]] Point3 at(double val) const noexcept
    {
      return Origin + (Direction * val);
    }
  };

  private:
  int width = 600;  // NOLINT
  int height = 400;  // NOLINT

  std::vector<std::uint8_t> rlPixels;
  std::vector<Colour> pixels;

  [[nodiscard]] Colour RayColour(const Ray& ray) const;

  public:
  void ResizeViewport(int width, int height);

  [[nodiscard]] const std::vector<std::uint8_t>& GetRGBAData();
  // const std::span<std::uint8_t> GetPixelData() const;
  void Render();
  [[nodiscard]] const std::vector<Colour>& GetPixelData() const { return pixels; }
};
