#pragma once

#include "random.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>

namespace softrays {

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

  [[nodiscard]] constexpr double LengthSquared() const noexcept
  {
    return (x * x) + (y * y) + (z * z);
  }

  [[nodiscard]] constexpr double Length() const noexcept
  {
    return std::sqrt(LengthSquared());
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

  [[nodiscard]] constexpr Vec3 operator+(double val) const noexcept
  {
    return {.x = val + x, .y = val + y, .z = val + z};
  }

  [[nodiscard]] constexpr Vec3 operator-(double val) const noexcept
  {
    return {.x = x - val, .y = y - val, .z = z - val};
  }

  [[nodiscard]] constexpr Vec3 operator*(double val) const noexcept
  {
    return {.x = val * x, .y = val * y, .z = val * z};
  }

  [[nodiscard]] constexpr Vec3 operator/(double val) const noexcept
  {
    return (*this) * (1 / val);
  }

  [[nodiscard]] constexpr double Dot(const Vec3& vec) const noexcept
  {
    return (x * vec.x)
        + (y * vec.y)
        + (z * vec.z);
  }

  [[nodiscard]] constexpr Vec3 Cross(const Vec3& vec) const noexcept
  {
    return {.x = (y * vec.z) - (z * vec.y),
        .y = (z * vec.x) - (x * vec.z),
        .z = (x * vec.y) - (y * vec.x)};
  }

  [[nodiscard]] constexpr Vec3 UnitVector() const noexcept
  {
    return *this / Length();
  }

  [[nodiscard]] static Vec3 RandomUnitVector() noexcept
  {
    constexpr auto min_len = 1e-160;
    // NOTE: this was in the book, but surely we are better off getting a random vector and just normalising it???
    while (true) {
      const auto rand = Vec3::Random(-1.0, 1.0);
      const auto lensq = rand.LengthSquared();
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

  [[nodiscard]] constexpr bool NearZero() const noexcept
  {
    // Return true if the vector is close to zero in all dimensions.
    // TODO: rather use std::numeric_limits<double>::min?
    constexpr auto near_zero = 1e-8;
    return (std::fabs(x) < near_zero) && (std::fabs(y) < near_zero) && (std::fabs(z) < near_zero);
  }

  [[nodiscard]] constexpr Vec3 Reflect(const Vec3& normal) const noexcept
  {
    return *this - normal * Dot(normal) * 2;
  }

  [[nodiscard]] constexpr Vec3 Refract(const Vec3& normal, double etai_over_etat) const noexcept
  {
    const auto cos_theta = std::fmin((-*this).Dot(normal), 1.0);
    const Vec3 r_out_perp = (*this + normal * cos_theta) * etai_over_etat;
    const Vec3 r_out_parallel = normal * (-std::sqrt(std::fabs(1.0 - r_out_perp.LengthSquared())));
    return r_out_perp + r_out_parallel;
  }

  [[nodiscard]] static Vec3 RandomInUnitDisk() noexcept
  {
    // TODO: again, surely there's a better way than looping
    while (true) {
      const auto rand = Vec3{.x = RandomDouble(-1, 1), .y = RandomDouble(-1, 1), .z = 0};
      if (rand.LengthSquared() < 1)
        return rand;
    }
  }

  [[nodiscard]] double operator[](this auto& self, std::size_t idx)
  {
    return idx == 0 ? self.x : (idx == 1 ? self.y : self.z);
  }
};

// point3 is just an alias for vec3, but useful for geometric clarity in the code.
using Point3 = Vec3;

using Colour = Vec3;

// Constants
constexpr auto Infinity = std::numeric_limits<double>::infinity();
constexpr double Pi = std::numbers::pi;
constexpr auto DegreesToRadiansFactor = Pi / 180.0;

// Utility Functions
[[nodiscard]] inline double DegreesToRadians(double degrees)
{
  return degrees * DegreesToRadiansFactor;
}

[[nodiscard]] inline Vec3 RandomOnHemisphere(const Vec3& normal) noexcept
{
  const Vec3 on_unit_sphere = Vec3::RandomUnitVector();
  // In the same hemisphere as the normal
  return (on_unit_sphere.Dot(normal) > 0.0) ? on_unit_sphere : -on_unit_sphere;
}

[[nodiscard]] inline Vec3 RandomInUnitSquare()
{
  constexpr auto offset = 0.5;
  // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
  return Vec3(RandomDouble() - offset, RandomDouble() - offset, 0);
}

struct Interval {
  double Min = Infinity;
  double Max = -Infinity;

  [[nodiscard]] static Interval FromIntervals(const Interval& a, const Interval& b)
  {
    // Create the interval tightly enclosing the two input intervals.
    return {.Min = a.Min <= b.Min ? a.Min : b.Min,
        .Max = a.Max >= b.Max ? a.Max : b.Max};
  }

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

  [[nodiscard]] Interval Expand(double delta) const noexcept
  {
    const auto padding = delta / 2.0;
    return {.Min = Min - padding, .Max = Max + padding};
  }

  const static Interval Empty;
  const static Interval Universe;
};

inline const Interval Interval::Empty = Interval{.Min = +Infinity, .Max = -Infinity};
inline const Interval Interval::Universe = Interval{.Min = -Infinity, .Max = Infinity};

struct Ray {
  Point3 Origin;
  Vec3 Direction;

  [[nodiscard]] Point3 At(double val) const noexcept
  {
    return Origin + (Direction * val);
  }
};

};
