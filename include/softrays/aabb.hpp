#pragma once
#include "math.hpp"

namespace softrays {
struct AABB {
  public:
  Interval X{};
  Interval Y{};
  Interval Z{};

  AABB() noexcept = default;

  AABB(const Interval& x, const Interval& y, const Interval& z) noexcept
      : X(x), Y(y), Z(z) { }

  AABB(const AABB& box0, const AABB& box1) noexcept : X(Interval::FromIntervals(box0.X, box1.X)),
                                                      Y(Interval::FromIntervals(box0.Y, box1.Y)),
                                                      Z(Interval::FromIntervals(box0.Z, box1.Z))
  {
  }

  // Treat the two points a and b as extrema for the bounding box, so we don't require a
  // particular minimum/maximum coordinate order.
  AABB(const Point3& a, const Point3& b)
      : X((a.x <= b.x) ? Interval{a.x, b.x} : Interval{b.x, a.x}),
        Y((a.y <= b.y) ? Interval{a.y, b.y} : Interval{b.y, a.y}),
        Z((a.z <= b.z) ? Interval{a.z, b.z} : Interval{b.z, a.z})
  {
  }

  [[nodiscard]] const Interval& AxisInterval(std::size_t axis) const
  {
    return axis == 1 ? Y : (axis == 2 ? Z : X);
  }

  [[nodiscard]] bool Hit(const Ray& r, Interval ray_t) const
  {
    const Point3& ray_orig = r.Origin;
    const Vec3& ray_dir = r.Direction;

    for (std::size_t axis = 0; axis < 3; axis++) {
      const Interval& ax = AxisInterval(axis);
      const double adinv = 1.0 / ray_dir[axis];

      const auto ray_orig_axis = ray_orig[axis];
      const auto t_0 = (ax.Min - ray_orig_axis) * adinv;
      const auto t_1 = (ax.Max - ray_orig_axis) * adinv;

      const auto [min, max] = std::minmax(t_0, t_1);
      ray_t.Min = min > ray_t.Min ? min : ray_t.Min;
      ray_t.Max = max < ray_t.Max ? max : ray_t.Max;
      // if (t_0 < t_1) {
      //   if (t_0 > ray_t.Min)
      //     ray_t.Min = t_0;
      //   if (t_1 < ray_t.Max)
      //     ray_t.Max = t_1;
      // } else {
      //   if (t_1 > ray_t.Min)
      //     ray_t.Min = t_1;
      //   if (t_0 < ray_t.Max)
      //     ray_t.Max = t_0;
      // }

      if (ray_t.Max <= ray_t.Min)
        return false;
    }
    return true;
  }

  // TODO: optimise this
  [[nodiscard]] std::size_t LongestAxis() const noexcept
  {
    // Returns the index of the longest axis of the bounding box.

    if (X.Size() > Y.Size())
      return X.Size() > Z.Size() ? 0 : 2;
    else
      return Y.Size() > Z.Size() ? 1 : 2;
  }

  static const AABB Empty;
  static const AABB Universe;
};

const inline AABB AABB::Empty = AABB(Interval::Empty, Interval::Empty, Interval::Empty);
const inline AABB AABB::Universe = AABB(Interval::Universe, Interval::Universe, Interval::Universe);
}
