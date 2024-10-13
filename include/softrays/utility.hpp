#pragma once

#include "math.hpp"

#include <cmath>
#include <iostream>
#include <memory>
#include <ostream>
#include <vector>

namespace softrays {

struct HitData {
  Point3 Location{};
  Vec3 Normal{};
  double Time{};
  bool FrontFace{};
  std::shared_ptr<struct MaterialBase> Material;

  // TODO: do we really want this here?
  void SetFaceNormal(const Ray& ray, const Vec3& outward_normal)
  {
    // Sets the hit record normal vector.
    // NOTE: the parameter `outward_normal` is assumed to have unit length.

    FrontFace = ray.Direction.Dot(outward_normal) < 0;
    Normal = FrontFace ? outward_normal : -outward_normal;
  }
};

// TODO: do we want this? or do we go with a std::variant, or something else?
class Hittable {
  public:
  Hittable() = default;
  Hittable(const Hittable&) = default;
  Hittable(Hittable&&) = default;
  Hittable& operator=(const Hittable&) = default;
  Hittable& operator=(Hittable&&) = default;
  virtual ~Hittable() = default;
  [[nodiscard]] virtual bool Hit(const Ray& ray, Interval ray_time, HitData& hit) const = 0;
};

// TODO: I REALLY HATE THIS, replace this ASAP
// should really be using a spatial data structure
class HittableList : public Hittable {
  private:
  std::vector<std::shared_ptr<Hittable>> Objects;

  public:
  void Clear() { Objects.clear(); }
  void Add(std::shared_ptr<Hittable>&& object)
  {
    Objects.push_back(std::move(object));
  }

  [[nodiscard]] bool Hit(const Ray& ray, Interval ray_time, HitData& hit) const override
  {
    HitData temp_hit{};
    bool hit_anything = false;
    double closest_so_far = ray_time.Max;

    for (const auto& object : Objects) {
      if (object->Hit(ray, {.Min = ray_time.Min, .Max = closest_so_far}, temp_hit)) {
        closest_so_far = temp_hit.Time;
        hit_anything = true;
        hit = temp_hit;
      }
    }
    return hit_anything;
  }
};

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

// Vector Utility Functions
inline std::ostream& operator<<(std::ostream& out, const Vec3& vec)
{
  return out << vec.x << ' ' << vec.y << ' ' << vec.z;
}

}
