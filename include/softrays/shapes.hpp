#pragma once

#include "raytracer.hpp"
#include <memory>
#include <utility>

class Sphere : public RayTracer::Hittable {
  private:
  Point3 Center;
  double Radius;
  std::shared_ptr<RayTracer::Material> Material;

  public:
  Sphere(const Point3& center, double radius, std::shared_ptr<RayTracer::Material>&& mat) noexcept
      : Center(center), Radius(std::fmax(0, radius)), Material(std::move(mat))
  {
  }

  [[nodiscard]] bool Hit(const RayTracer::Ray& ray, Interval ray_time, RayTracer::HitData& hit) const override
  {
    const Vec3 o_c = Center - ray.Origin;
    const auto a = ray.Direction.length_squared();
    const auto hyp = ray.Direction.dot(o_c);
    const auto c_comp = o_c.length_squared() - (Radius * Radius);
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
    hit.Location = ray.at(hit.Time);
    const Vec3 outward_normal = (hit.Location - Center) / Radius;
    hit.SetFaceNormal(ray, outward_normal);
    hit.Material = Material;
    return true;
  }
};
