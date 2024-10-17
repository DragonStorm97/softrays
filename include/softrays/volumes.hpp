#pragma once

#include <iostream>
#include <utility>

#include "hittable.hpp"
#include "material.hpp"
#include "texture.hpp"

namespace softrays {

class ConstantMedium : public Hittable {
  public:
  ConstantMedium(std::shared_ptr<Hittable> boundary, double density, std::shared_ptr<Texture> tex)
      : boundary(std::move(boundary)), neg_inv_density(-1 / density), phase_function(std::make_shared<Isotropic>(tex))
  {
  }

  ConstantMedium(std::shared_ptr<Hittable> boundary, double density, const Colour& albedo)
      : boundary(std::move(boundary)), neg_inv_density(-1 / density), phase_function(std::make_shared<Isotropic>(albedo))
  {
  }

  [[nodiscard]] const AABB& BoundingBox() const noexcept override { return boundary->BoundingBox(); }
  [[nodiscard]] bool Hit(const Ray& ray, Interval ray_time, HitData& hit) const override
  {
    HitData rec1;
    HitData rec2;

    if (!boundary->Hit(ray, Interval::Universe, rec1))
      return false;

    constexpr static auto min_offset = 0.0001;
    if (!boundary->Hit(ray, Interval(rec1.Time + min_offset, Infinity), rec2))
      return false;

    if (rec1.Time < ray_time.Min)
      rec1.Time = ray_time.Min;
    if (rec2.Time > ray_time.Max)
      rec2.Time = ray_time.Max;

    if (rec1.Time >= rec2.Time)
      return false;

    if (rec1.Time < 0)
      rec1.Time = 0;

    auto ray_length = ray.Direction.Length();
    auto distance_inside_boundary = (rec2.Time - rec1.Time) * ray_length;
    auto hit_distance = neg_inv_density * std::log(RandomDouble());

    if (hit_distance > distance_inside_boundary)
      return false;

    hit.Time = rec1.Time + hit_distance / ray_length;
    hit.Location = ray.At(hit.Time);

    // NOTE: this isn't fantastic, but at least we can have a textured surface on a volume
    HitData boundaryHitTest;
    if (boundary->Hit(ray, ray_time, boundaryHitTest)) {
      hit.U = boundaryHitTest.U;
      hit.V = boundaryHitTest.V;
    }

    hit.Normal = Vec3(1, 0, 0);  // arbitrary
    hit.FrontFace = true;  // also arbitrary
    hit.Material = phase_function;

    return true;
  }

  private:
  std::shared_ptr<Hittable> boundary;
  double neg_inv_density;
  std::shared_ptr<MaterialBase> phase_function;
};

}
