#pragma once

#include <random>

[[nodiscard]] inline double RandomDouble()
{
  static std::random_device rd;
  static std::mt19937 generator{rd()};
  static std::uniform_real_distribution<double> distribution(0.0, 1.0);
  return distribution(generator);
}

// Returns a random real in [min,max).
[[nodiscard]] inline double RandomDouble(double min, double max)
{
  return min + ((max - min) * RandomDouble());
}

// Returns a random int in [min,max).
// TODO: maybe rather use a std::uniform_real_distribution<int>??
[[nodiscard]] inline int RandomInt(int min, int max)
{
  return static_cast<int>(RandomDouble(min, max));
}
