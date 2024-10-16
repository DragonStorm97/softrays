#pragma once

#include <utility>

#include "hittable.hpp"
#include "math.hpp"

// NOTE: I really don't think we need even more indirection,
// an object's transform really should be a property of the object instead of wrapping the object in transforms like this
// TODO: add transforms to shapes, and rather transform from within their Hit function

namespace softrays {

// TODO: Shapes have an origin/location, do we really need to translate them like this?
class Translate : public Hittable {
  public:
  Translate(std::shared_ptr<Hittable> obj, Vec3 offset)
      : object(std::move(obj)), offset(offset), bbox(object->BoundingBox() + offset)
  {
  }

  [[nodiscard]] const AABB& BoundingBox() const noexcept override { return bbox; }

  [[nodiscard]] bool Hit(const Ray& ray, Interval ray_time, HitData& hit) const override
  {
    // Move the ray backwards by the offset
    const Ray offset_r(ray.Origin - offset, ray.Direction);

    // Determine whether an intersection exists along the offset ray (and if so, where)
    if (!object->Hit(offset_r, ray_time, hit))
      return false;

    // Move the intersection point forwards by the offset
    hit.Location += offset;

    return true;
  }

  private:
  std::shared_ptr<Hittable> object;
  Vec3 offset;
  AABB bbox;
};

class Rotate_Y : public Hittable {
  public:
  Rotate_Y(std::shared_ptr<Hittable> obj, double angle) : object(std::move(obj))
  {
    const auto radians = DegreesToRadians(angle);
    sin_theta = std::sin(radians);
    cos_theta = std::cos(radians);
    bbox = object->BoundingBox();

    Point3 min(Infinity, Infinity, Infinity);
    Point3 max(-Infinity, -Infinity, -Infinity);

    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        for (int k = 0; k < 2; k++) {
          const auto x = i * bbox.X.Max + (1 - i) * bbox.X.Min;
          const auto y = j * bbox.Y.Max + (1 - j) * bbox.Y.Min;
          const auto z = k * bbox.Z.Max + (1 - k) * bbox.Z.Min;

          const auto newx = cos_theta * x + sin_theta * z;
          const auto newz = -sin_theta * x + cos_theta * z;

          const Vec3 tester{newx, y, newz};

          for (int c = 0; c < 3; c++) {
            min[c] = std::fmin(min[c], tester[c]);
            max[c] = std::fmax(max[c], tester[c]);
          }
        }
      }
    }

    bbox = AABB(min, max);
  }

  [[nodiscard]] bool Hit(const Ray& ray, Interval ray_time, HitData& hit) const override
  {
    // Transform the ray from world space to object space.

    auto origin = Point3(
        (cos_theta * ray.Origin.x) - (sin_theta * ray.Origin.z),
        ray.Origin.y,
        (sin_theta * ray.Origin.x) + (cos_theta * ray.Origin.z));

    auto direction = Vec3{(cos_theta * ray.Direction.x) - (sin_theta * ray.Direction.z),
        ray.Direction.y,
        (sin_theta * ray.Direction.x) + (cos_theta * ray.Direction.z)};

    Ray rotated_r(origin, direction);

    // Determine whether an intersection exists in object space (and if so, where).

    if (!object->Hit(rotated_r, ray_time, hit))
      return false;

    // Transform the intersection from object space back to world space.

    hit.Location = Point3{(cos_theta * hit.Location.x) + (sin_theta * hit.Location.z),
        hit.Location.y,
        (-sin_theta * hit.Location.x) + (cos_theta * hit.Location.z)};

    hit.Normal = Vec3{(cos_theta * hit.Normal.x) + (sin_theta * hit.Normal.z),
        hit.Normal.y,
        (-sin_theta * hit.Normal.x) + (cos_theta * hit.Normal.z)};

    return true;
  }

  [[nodiscard]] const AABB& BoundingBox() const noexcept override { return bbox; }

  private:
  std::shared_ptr<Hittable> object;
  double sin_theta;
  double cos_theta;
  AABB bbox;
};

}
