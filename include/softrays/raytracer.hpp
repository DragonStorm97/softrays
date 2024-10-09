#pragma once

#include "utility.hpp"
#include <cstdint>
#include <memory>

class RayTracer {
  public:
  struct Ray {
    Point3 Origin;
    Vec3 Direction;

    [[nodiscard]] Point3 at(double val) const noexcept
    {
      return Origin + (Direction * val);
    }
  };

  struct HitData {
    Point3 Location;
    Vec3 Normal;
    double Time;
    bool FrontFace;

    // TODO: do we really want this here?
    void SetFaceNormal(const Ray& ray, const Vec3& outward_normal)
    {
      // Sets the hit record normal vector.
      // NOTE: the parameter `outward_normal` is assumed to have unit length.

      FrontFace = ray.Direction.dot(outward_normal) < 0;
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
    [[nodiscard]] virtual bool Hit(const Ray& ray, double ray_tmin, double ray_tmax, HitData& hit) const = 0;
  };

  // TODO: I REALLY HATE THIS, replace this ASAP
  // if we ignore everything else, why does it implement Hittable anyway?
  class HittableList : public Hittable {
private:
    std::vector<std::shared_ptr<Hittable>> Objects;

public:
    void Clear() { Objects.clear(); }
    void Add(std::shared_ptr<Hittable>&& object)
    {
      Objects.push_back(object);
    }

    [[nodiscard]] bool Hit(const Ray& ray, double ray_tmin, double ray_tmax, HitData& hit) const override
    {
      RayTracer::HitData temp_hit{};
      bool hit_anything = false;
      double closest_so_far = ray_tmax;

      for (const auto& object : Objects) {
        if (object->Hit(ray, ray_tmin, closest_so_far, temp_hit)) {
          closest_so_far = temp_hit.Time;
          hit_anything = true;
          hit = temp_hit;
        }
      }
      return hit_anything;
    }
  };
  Point3 CameraPosition{0, 0, 0};

  private:
  int width = 600;  // NOLINT
  int height = 400;  // NOLINT

  HittableList World;
  std::vector<std::uint8_t> rlPixels;
  std::vector<Colour> pixels;

  [[nodiscard]] Colour RayColour(const Ray& ray, const class Hittable& World) const;

  public:
  [[nodiscard]] HittableList& GetWorld() { return World; }
  void ResizeViewport(int width, int height);

  [[nodiscard]] const std::vector<std::uint8_t>& GetRGBAData();
  // const std::span<std::uint8_t> GetPixelData() const;
  void Render();
  [[nodiscard]] const std::vector<Colour>& GetPixelData() const { return pixels; }
};
