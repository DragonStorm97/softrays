#pragma once
#include "random.hpp"
#include "utility.hpp"

namespace softrays {
class BVH : public Hittable {
  public:
  // TODO: replace with span
  // TODO: surely we can don't have to sort the list from the nodes...
  BVH(std::vector<std::shared_ptr<Hittable>>& objects, std::size_t start, std::size_t end) : bbox(AABB::Empty)
  {
    // Build the bounding box of the span of source objects.

    for (size_t object_index = start; object_index < end; object_index++)
      bbox = AABB(bbox, objects[object_index]->BoundingBox());

    const std::size_t axis = bbox.LongestAxis();

    const auto comparator = (axis == 0) ? box_x_compare
        : (axis == 1)                   ? box_y_compare
                                        : box_z_compare;
    const auto object_span = end - start;

    if (object_span == 1) {
      left = right = objects[start];
    } else if (object_span == 2) {
      left = objects[start];
      right = objects[start + 1];
    } else {
      std::sort(std::begin(objects) + start, std::begin(objects) + end, comparator);
      auto mid = start + object_span / 2;
      left = std::make_shared<BVH>(objects, start, mid);
      right = std::make_shared<BVH>(objects, mid, end);
    }

    bbox = AABB(left->BoundingBox(), right->BoundingBox());
  }

  // TODO: Do we really need to take a copy here??
  BVH(HittableList list) : BVH(list.GetObjects(), 0, list.GetObjects().size())
  {
    // There's a C++ subtlety here. This constructor (without span indices) creates an
    // implicit copy of the Hittable list, which we will modify. The lifetime of the copied
    // list only extends until this constructor exits. That's OK, because we only need to
    // persist the resulting bounding volume hierarchy.
  }

  bool Hit(const Ray& r, Interval ray_t, HitData& rec) const override
  {
    if (!bbox.Hit(r, ray_t))
      return false;

    bool hit_left = left->Hit(r, ray_t, rec);
    bool hit_right = right->Hit(r, Interval(ray_t.Min, hit_left ? rec.Time : ray_t.Max), rec);

    return hit_left || hit_right;
  }

  [[nodiscard]] const AABB& BoundingBox() const override { return bbox; }

  private:
  std::shared_ptr<Hittable> left;
  std::shared_ptr<Hittable> right;
  AABB bbox;

  static bool box_compare(const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b, std::size_t axis_index)
  {
    auto a_axis_interval = a->BoundingBox().AxisInterval(axis_index);
    auto b_axis_interval = b->BoundingBox().AxisInterval(axis_index);
    return a_axis_interval.Min < b_axis_interval.Min;
  }

  static bool box_x_compare(const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b)
  {
    return box_compare(a, b, 0);
  }

  static bool box_y_compare(const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b)
  {
    return box_compare(a, b, 1);
  }

  static bool box_z_compare(const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b)
  {
    return box_compare(a, b, 2);
  }
};
}
