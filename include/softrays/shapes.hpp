#pragma once

#include "hittable.hpp"
#include "material.hpp"

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
    GetUV(hit.Location, hit.U, hit.V);
    hit.Material = Material;
    return true;
  }

  static void GetUV(const Point3& loc, double& u, double& v)
  {
    // loc: a given point on the sphere of radius one, centered at the origin.
    // u: returned value [0,1] of angle around the Y axis from X=-1.
    // v: returned value [0,1] of angle from Y=-1 to Y=+1.
    //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
    //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
    //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>

    auto theta = std::acos(-loc.y);
    auto phi = std::atan2(-loc.z, loc.x) + Pi;

    u = phi / (2 * Pi);
    v = theta / Pi;
  }
};
}
