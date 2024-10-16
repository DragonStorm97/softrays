#pragma once

#include "hittable.hpp"
#include "material.hpp"

#include <iostream>
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
  Sphere(const Point3& center, double radius, std::shared_ptr<MaterialBase> mat) noexcept
      : Center(center), Radius(std::fmax(0, radius)), Material(std::move(mat)), bbox(Center - radius, Center + radius)
  {
  }

  [[nodiscard]] const AABB& BoundingBox() const noexcept override
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
    GetUV(hit.Normal, hit.U, hit.V);
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

    u = phi / (2.0 * Pi);
    v = theta / Pi;
  }
};

class Quad : public Hittable {
  public:
  // NOTE: Q is the position of bottoom left corner;
  //   u is the "width" vector from Q to bottom right corner;
  //   v is the "height" vector from Q to the top left corner
  Quad(const Point3& pos, const Vec3& u, const Vec3& v, std::shared_ptr<MaterialBase> mat)
      : Q(pos), u(u), v(v), mat(std::move(mat))
  {
    auto n = u.Cross(v);
    normal = n.UnitVector();
    D = normal.Dot(Q);
    w = n / n.Dot(n);
    SetBoundingBox();
  }

  virtual void SetBoundingBox()
  {
    // Compute the bounding box of all four vertices.
    auto bbox_diagonal1 = AABB(Q, Q + u + v);
    auto bbox_diagonal2 = AABB(Q + u, Q + v);
    bbox = AABB(bbox_diagonal1, bbox_diagonal2);
  }

  [[nodiscard]] const AABB& BoundingBox() const noexcept override
  {
    return bbox;
  }

  [[nodiscard]] bool Hit(const Ray& ray, Interval ray_time, HitData& hit) const override
  {
    auto denom = normal.Dot(ray.Direction);

    // No hit if the ray is parallel to the plane.
    constexpr static auto minimum = 1e-8;
    if (std::fabs(denom) < minimum)
      return false;

    // Return false if the hit point parameter t is outside the ray Interval.
    auto time = (D - normal.Dot(ray.Origin)) / denom;
    if (!ray_time.Contains(time))
      return false;

    // Determine if the hit point lies within the planar shape using its plane coordinates.
    const auto intersection = ray.At(time);
    Vec3 planar_hitpt_vector = intersection - Q;
    auto alpha = w.Dot(planar_hitpt_vector.Cross(v));
    auto beta = w.Dot(u.Cross(planar_hitpt_vector));

    if (!IsInterior(alpha, beta, hit))
      return false;

    // Ray hits the 2D shape; set the rest of the hit record and return true.    hit.Time = time;
    hit.Time = time;
    hit.Location = intersection;
    hit.Material = mat;
    hit.SetFaceNormal(ray, normal);

    return true;
  }

  [[nodiscard]] virtual bool IsInterior(double a, double b, HitData& hit) const noexcept
  {
    const auto unit_interval = Interval(0, 1);
    // Given the hit point in plane coordinates, return false if it is outside the
    // primitive, otherwise set the hit record UV coordinates and return true.

    if (!unit_interval.Contains(a) || !unit_interval.Contains(b))
      return false;

    hit.U = a;
    hit.V = b;
    return true;
  }

  protected:
  Point3 Q;
  Vec3 u, v, w;
  std::shared_ptr<MaterialBase> mat;
  AABB bbox;
  Vec3 normal;
  double D{};
};

inline std::shared_ptr<HittableList> MakeBoxQuadList(const Point3& a, const Point3& b, std::shared_ptr<MaterialBase> mat)
{
  // Returns the 3D box (six sides) that contains the two opposite vertices a & b.

  auto sides = std::make_shared<HittableList>();

  // Construct the two opposite vertices with the minimum and maximum coordinates.
  auto min = Point3(std::fmin(a.x, b.x), std::fmin(a.y, b.y), std::fmin(a.z, b.z));
  auto max = Point3(std::fmax(a.x, b.x), std::fmax(a.y, b.y), std::fmax(a.z, b.z));

  auto dx = Vec3(max.x - min.x, 0, 0);
  auto dy = Vec3(0, max.y - min.y, 0);
  auto dz = Vec3(0, 0, max.z - min.z);

  sides->Add(make_shared<Quad>(Point3(min.x, min.y, max.z), dx, dy, mat));  // front
  sides->Add(make_shared<Quad>(Point3(max.x, min.y, max.z), -dz, dy, mat));  // right
  sides->Add(make_shared<Quad>(Point3(max.x, min.y, min.z), -dx, dy, mat));  // back
  sides->Add(make_shared<Quad>(Point3(min.x, min.y, min.z), dz, dy, mat));  // left
  sides->Add(make_shared<Quad>(Point3(min.x, max.y, max.z), dx, -dz, mat));  // top
  sides->Add(make_shared<Quad>(Point3(min.x, min.y, min.z), dx, dz, mat));  // bottom

  return sides;
}

class Triangle : public Quad {
  public:
  Triangle(const Point3& o, const Vec3& aa, const Vec3& ab, std::shared_ptr<MaterialBase> m)
      : Quad(o, aa, ab, std::move(m))
  {
  }

  [[nodiscard]] bool IsInterior(double a, double b, HitData& rec) const noexcept override
  {
    if ((a < 0) || (b < 0) || (a + b > 1))
      return false;

    rec.U = a;
    rec.V = b;
    return true;
  }
};

class Ellipse : public Quad {
  public:
  Ellipse(
      const Point3& center, const Vec3& side_A, const Vec3& side_B, std::shared_ptr<MaterialBase> m) : Quad(center, side_A, side_B, m)
  {
  }

  void SetBoundingBox() override
  {
    const auto half_u = u / 2.0;
    const auto half_v = v / 2.0;
    bbox = AABB(Q - half_u - half_v, Q + half_u + half_v);
  }

  // NOTE: had to modify a & b, as the quad works off of the bottom corner to U & V, so we have to interpolate it
  [[nodiscard]] bool IsInterior(double a, double b, HitData& rec) const noexcept override
  {
    const auto offset_a = a * 2.0 - 1.0;
    const auto offset_b = b * 2.0 - 1.0;
    if ((offset_a * offset_a + offset_b * offset_b) > 1)
      return false;

    rec.U = a;
    rec.V = b;
    return true;
  }
};

class Annulus : public Quad {
  public:
  Annulus(
      const Point3& center, const Vec3& side_A, const Vec3& side_B, double _inner,
      std::shared_ptr<MaterialBase> m)
      : Quad(center, side_A, side_B, std::move(m)), inner(_inner)
  {
  }

  void SetBoundingBox() override
  {
    const auto half_u = u / 2.0;
    const auto half_v = v / 2.0;
    bbox = AABB(Q - half_u - half_v, Q + half_u + half_v);
  }

  // NOTE: had to modify a & b, as the quad works off of the bottom corner to U & V, so we have to interpolate it
  [[nodiscard]] bool IsInterior(double a, double b, HitData& rec) const noexcept override
  {
    const auto offset_a = a * 2.0 - 1.0;
    const auto offset_b = b * 2.0 - 1.0;
    const auto center_dist = sqrt(offset_a * offset_a + offset_b * offset_b);
    if ((center_dist < inner) || (center_dist > 1))
      return false;

    rec.U = a;
    rec.V = b;
    return true;
  }

  private:
  double inner;
};
}
