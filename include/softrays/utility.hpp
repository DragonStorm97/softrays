#pragma once

#include <cmath>
#include <ostream>
struct Vec3 {
  double x;
  double y;
  double z;

  [[nodiscard]] constexpr Vec3 operator-() const noexcept
  {
    return {-x, -y, -z};
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
    return {x + vec.x, y + vec.y, z + vec.z};
  }

  [[nodiscard]] constexpr Vec3 operator-(const Vec3& vec) const noexcept
  {
    return {x - vec.x, y - vec.y, z - vec.z};
  }

  [[nodiscard]] constexpr Vec3 operator*(const Vec3& vec) const noexcept
  {
    return {x * vec.x, y * vec.y, z * vec.z};
  }

  [[nodiscard]] constexpr Vec3 operator*(double val) const noexcept
  {
    return {val * x, val * y, val * z};
  }

  [[nodiscard]] constexpr Vec3 operator/(double val) const noexcept
  {
    return (*this) * (1 / val);
  }

  [[nodiscard]] constexpr double dot(const Vec3& vec) const noexcept
  {
    return x * vec.x
        + y * vec.y
        + z * vec.z;
  }

  [[nodiscard]] constexpr Vec3 cross(const Vec3& vec) const noexcept
  {
    return {y * vec.z - z * vec.y,
        z * vec.x - x * vec.z,
        x * vec.y - y * vec.x};
  }

  [[nodiscard]] constexpr Vec3 unit_vector() const noexcept
  {
    return *this / length();
  }
};

// point3 is just an alias for vec3, but useful for geometric clarity in the code.
using Point3 = Vec3;

// Vector Utility Functions

inline std::ostream& operator<<(std::ostream& out, const Vec3& vec)
{
  return out << vec.x << ' ' << vec.y << ' ' << vec.z;
}

using Colour = Vec3;

constexpr auto ColourByteFactor = 255.999;

constexpr inline void write_color(std::ostream& out, const Colour& pixel_colour)
{
  const auto r = pixel_colour.x;
  const auto g = pixel_colour.x;
  const auto b = pixel_colour.x;

  // Translate the [0,1] component values to the byte range [0,255].
  const auto rbyte = static_cast<std::uint8_t>(ColourByteFactor * r);
  const auto gbyte = static_cast<std::uint8_t>(ColourByteFactor * g);
  const auto bbyte = static_cast<std::uint8_t>(ColourByteFactor * b);

  // Write out the pixel color components.
  out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

struct Ray {
  Point3 Origin;
  Vec3 Direction;
};
