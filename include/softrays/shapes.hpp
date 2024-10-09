#pragma once

#include <raytracer.hpp>

class Sphere : public RayTracer::Hittable {
  private:
  Point3 Center;
  double Radius;

  public:
  Sphere(const Point3& center, double radius) noexcept
      : Center(center), Radius(std::fmax(0, radius)) { }

  [[nodiscard]] bool Hit(const RayTracer::Ray& ray, double ray_tmin, double ray_tmax, RayTracer::HitData& hit) const override
  {
    const Vec3 o_c = Center - ray.Origin;
    const auto a = ray.Direction.length_squared();
    const auto h = ray.Direction.dot(o_c);
    const auto c = o_c.length_squared() - (Radius * Radius);
    const auto discriminant = (h * h) - (a * c);

    if (discriminant < 0) {
      return false;
    }

    const auto sqrtd = std::sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (h - sqrtd) / a;
    if (root <= ray_tmin || ray_tmax <= root) {
      root = (h + sqrtd) / a;
      if (root <= ray_tmin || ray_tmax <= root)
        return false;
    }

    hit.Time = root;
    hit.Location = ray.at(hit.Time);
    const Vec3 outward_normal = (hit.Location - Center) / Radius;
    hit.SetFaceNormal(ray, outward_normal);

    return true;
  }
};
