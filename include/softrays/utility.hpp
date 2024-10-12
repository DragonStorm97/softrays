#pragma once

#include <cmath>
#include <iostream>
#include <ostream>
#include <vector>

constexpr double LinearToGamma(double linear_component)
{
  if (linear_component > 0)
    return std::sqrt(linear_component);

  return 0;
}

inline void StreamPPM(std::ostream& stream, int width, int height, const std::vector<std::uint8_t>& data)
{
  stream << "P3\n"
         << width << ' ' << height << "\n255\n";
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      const auto rl_pixel_start = static_cast<std::size_t>((y * width) + x) * 4;
      // Write out the pixel color components.
      stream << ' ' << data[rl_pixel_start] << ' ' << data[rl_pixel_start + 1] << ' ' << data[rl_pixel_start + 2] << '\n';
    }
  }
}

inline void PrintPPM(int width, int height, const std::vector<std::uint8_t>& data)
{
  StreamPPM(std::cout, width, height, data);
}

struct Dimension2d {
  int Width{};
  int Height{};
};
