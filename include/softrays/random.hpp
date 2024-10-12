#pragma once

#include <random>

[[nodiscard]] inline double RandomDouble()
{
  static std::random_device rd;
  static std::mt19937 generator{rd()};
  static std::uniform_real_distribution<double> distribution(0.0, 1.0);
  return distribution(generator);
}

[[nodiscard]] inline double RandomDouble(double min, double max)
{
  // Returns a random real in [min,max).
  return min + ((max - min) * RandomDouble());
}
