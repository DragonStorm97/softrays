#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <numbers>
#include <ostream>
#include <random>
#include <vector>

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

struct Vec3 {
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;

  [[nodiscard]] constexpr Vec3 operator-() const noexcept
  {
    return {.x = -x, .y = -y, .z = -z};
  }
  constexpr void operator+=(const Vec3& other) noexcept
  {
    x += other.x;
    y += other.y;
    z += other.z;
  }
  constexpr void operator*=(double val) noexcept
  {
    x *= val;
    y *= val;
    z *= val;
  }
  constexpr void operator/=(double val) noexcept
  {
    *this *= 1.0 / val;
  }
  [[nodiscard]] constexpr double length_squared() const noexcept
  {
    return (x * x) + (y * y) + (z * z);
  }
  [[nodiscard]] constexpr double length() const noexcept
  {
    return std::sqrt(length_squared());
  }

  [[nodiscard]] constexpr Vec3 operator+(const Vec3& vec) const noexcept
  {
    return {.x = x + vec.x, .y = y + vec.y, .z = z + vec.z};
  }

  [[nodiscard]] constexpr Vec3 operator-(const Vec3& vec) const noexcept
  {
    return {.x = x - vec.x, .y = y - vec.y, .z = z - vec.z};
  }

  [[nodiscard]] constexpr Vec3 operator*(const Vec3& vec) const noexcept
  {
    return {.x = x * vec.x, .y = y * vec.y, .z = z * vec.z};
  }

  [[nodiscard]] constexpr Vec3 operator*(double val) const noexcept
  {
    return {.x = val * x, .y = val * y, .z = val * z};
  }

  [[nodiscard]] constexpr Vec3 operator/(double val) const noexcept
  {
    return (*this) * (1 / val);
  }

  [[nodiscard]] constexpr double dot(const Vec3& vec) const noexcept
  {
    return (x * vec.x)
        + (y * vec.y)
        + (z * vec.z);
  }

  [[nodiscard]] constexpr Vec3 cross(const Vec3& vec) const noexcept
  {
    return {.x = (y * vec.z) - (z * vec.y),
        .y = (z * vec.x) - (x * vec.z),
        .z = (x * vec.y) - (y * vec.x)};
  }

  [[nodiscard]] constexpr Vec3 unit_vector() const noexcept
  {
    return *this / length();
  }

  [[nodiscard]] static Vec3 RandomUnitVector() noexcept
  {
    constexpr auto min_len = 1e-160;
    // NOTE: this was in the book, but surely we are better off getting a random vector and just normalising it???
    while (true) {
      const auto rand = Vec3::Random(-1.0, 1.0);
      const auto lensq = rand.length_squared();
      if (min_len < lensq && lensq <= 1) {
        return rand / std::sqrt(lensq);
      }
    }
    // NOTE: I tried this, but it did look different for some reason
    // return Vec3::Random().unit_vector();
  }

  [[nodiscard]] static Vec3 Random()
  {
    return Vec3(RandomDouble(), RandomDouble(), RandomDouble());
  }

  [[nodiscard]] static Vec3 Random(double min, double max)
  {
    return Vec3(RandomDouble(min, max), RandomDouble(min, max), RandomDouble(min, max));
  }
  [[nodiscard]] constexpr bool near_zero() const noexcept
  {
    // Return true if the vector is close to zero in all dimensions.
    constexpr auto near_zero = 1e-8;
    return (std::fabs(x) < near_zero) && (std::fabs(y) < near_zero) && (std::fabs(z) < near_zero);
  }
  [[nodiscard]] constexpr Vec3 Reflect(const Vec3& normal) const noexcept
  {
    return *this - normal * dot(normal) * 2;
  }
  [[nodiscard]] constexpr Vec3 Refract(const Vec3& normal, double etai_over_etat) const noexcept
  {
    auto cos_theta = std::fmin((-*this).dot(normal), 1.0);
    Vec3 r_out_perp = (*this + normal * cos_theta) * etai_over_etat;
    Vec3 r_out_parallel = normal * (-std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())));
    return r_out_perp + r_out_parallel;
  }
};

[[nodiscard]] inline Vec3 RandomOnHemisphere(const Vec3& normal) noexcept
{
  const Vec3 on_unit_sphere = Vec3::RandomUnitVector();
  // In the same hemisphere as the normal
  return (on_unit_sphere.dot(normal) > 0.0) ? on_unit_sphere : -on_unit_sphere;
}

[[nodiscard]] inline Vec3 RandomInUnitDisk() noexcept
{
  // TODO: again, surely there's a better way than looping
  while (true) {
    const auto rand = Vec3{.x = RandomDouble(-1, 1), .y = RandomDouble(-1, 1), .z = 0};
    if (rand.length_squared() < 1)
      return rand;
  }
}

// point3 is just an alias for vec3, but useful for geometric clarity in the code.
using Point3 = Vec3;

// Vector Utility Functions

inline std::ostream& operator<<(std::ostream& out, const Vec3& vec)
{
  return out << vec.x << ' ' << vec.y << ' ' << vec.z;
}

using Colour = Vec3;

// Constants
constexpr auto Infinity = std::numeric_limits<double>::infinity();
constexpr double Pi = std::numbers::pi;
constexpr auto DegreesToRadiansFactor = Pi / 180.0;

// Utility Functions

inline double DegreesToRadians(double degrees)
{
  return degrees * DegreesToRadiansFactor;
}

struct Interval {
  double Min = -Infinity;
  double Max = Infinity;

  [[nodiscard]] double Size() const noexcept
  {
    return Max - Min;
  }

  [[nodiscard]] bool Contains(double x) const noexcept
  {
    return Min <= x && x <= Max;
  }

  [[nodiscard]] bool Surrounds(double x) const noexcept
  {
    return Min < x && x < Max;
  }

  [[nodiscard]] double Clamp(double x) const noexcept
  {
    return std::clamp(x, Min, Max);
  }
  const static Interval Empty;
  const static Interval Universe;
};

inline const Interval Interval::Empty = Interval{};
inline const Interval Interval::Universe = Interval{.Min = -Infinity, .Max = Infinity};

constexpr double linear_to_gamma(double linear_component)
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
