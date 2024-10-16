#pragma once

#include "aabb.hpp"
#include "math.hpp"
#include <memory>

namespace softrays {
struct HitData {
  Point3 Location{};
  Vec3 Normal{};
  double Time{};
  bool FrontFace;
  double U{};
  double V{};

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
  [[nodiscard]] virtual const AABB& BoundingBox() const noexcept = 0;
  [[nodiscard]] virtual bool Hit(const Ray& ray, Interval ray_time, HitData& hit) const = 0;
};

// TODO: this should use the BVH directly
class HittableList : public Hittable {
  private:
  AABB bbox;

  std::vector<std::shared_ptr<Hittable>> Objects;

  public:
  HittableList() = default;
  HittableList(std::shared_ptr<Hittable> object) { Add(std::move(object)); }
  void Clear() { Objects.clear(); }
  void Add(std::shared_ptr<Hittable> object)
  {
    bbox = AABB(bbox, object->BoundingBox());
    Objects.push_back(std::move(object));
  }

  [[nodiscard]] auto& GetObjects() { return Objects; }
  [[nodiscard]] const auto& GetObjects() const { return Objects; }
  [[nodiscard]] const AABB& BoundingBox() const noexcept override { return bbox; }
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

}
