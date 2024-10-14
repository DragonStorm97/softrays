#pragma once

#include "material.hpp"
#include "utility.hpp"

#include <memory>
#include <utility>

namespace softrays {
class Sphere : public Hittable {
  private:
  Point3 Center;
  double Radius;
  std::shared_ptr<MaterialBase> Material;
  AABB bbox;

  public:
  Sphere(const Point3& center, double radius, std::shared_ptr<MaterialBase>&& mat) noexcept
      : Center(center), Radius(std::fmax(0, radius)), Material(std::move(mat)), bbox(Center - radius, Center + radius)
  {
  }

  [[nodiscard]] const AABB& BoundingBox() const override
  {
    return bbox;
  }

  [[nodiscard]] bool Hit(const Ray& ray, Interval ray_time, HitData& hit) const override
  {
    const Vec3 o_c = Center - ray.Origin;
    const auto a = ray.Direction.LengthSquared();
    const auto hyp = ray.Direction.Dot(o_c);
    const auto c_comp = o_c.LengthSquared() - (Radius * Radius);
    const auto discriminant = (hyp * hyp) - (a * c_comp);

    if (discriminant < 0) {
      return false;
    }

    const auto sqrtd = std::sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (hyp - sqrtd) / a;
    if (!ray_time.Surrounds(root)) {
      root = (hyp + sqrtd) / a;
      if (!ray_time.Surrounds(root))
        return false;
    }

    hit.Time = root;
    hit.Location = ray.At(hit.Time);
    const Vec3 outward_normal = (hit.Location - Center) / Radius;
    hit.SetFaceNormal(ray, outward_normal);
    hit.Material = Material;
    return true;
  }
};
}
