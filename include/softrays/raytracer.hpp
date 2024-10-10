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

  struct Material;

  struct HitData {
    Point3 Location;
    Vec3 Normal;
    double Time;
    bool FrontFace;
    std::shared_ptr<RayTracer::Material> Material;

    // TODO: do we really want this here?
    void SetFaceNormal(const Ray& ray, const Vec3& outward_normal)
    {
      // Sets the hit record normal vector.
      // NOTE: the parameter `outward_normal` is assumed to have unit length.

      FrontFace = ray.Direction.dot(outward_normal) < 0;
      Normal = FrontFace ? outward_normal : -outward_normal;
    }
  };

  struct Material {
    Material() = default;
    Material(const Material&) = default;
    Material(Material&&) = delete;
    Material& operator=(const Material&) = default;
    Material& operator=(Material&&) = delete;

    virtual ~Material() = default;
    [[nodiscard]] virtual bool Scatter([[maybe_unused]] const Ray& ray, [[maybe_unused]] const HitData& hit, [[maybe_unused]] Colour& attenuation, [[maybe_unused]] Ray& scattered) const
    {
      return false;
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
    [[nodiscard]] virtual bool Hit(const Ray& ray, Interval ray_time, HitData& hit) const = 0;
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

    [[nodiscard]] bool Hit(const Ray& ray, Interval ray_time, HitData& hit) const override
    {
      RayTracer::HitData temp_hit{};
      bool hit_anything = false;
      double closest_so_far = ray_time.Max;

      for (const auto& object : Objects) {
        if (object->Hit(ray, {ray_time.Min, closest_so_far}, temp_hit)) {
          closest_so_far = temp_hit.Time;
          hit_anything = true;
          hit = temp_hit;
        }
      }
      return hit_anything;
    }
  };

  Point3 CameraPosition{0, 0, 0};

  int MaxDepth = 50;  // NOLINT
  double vfov = 90;  // Vertical view angle (field of view)

  Point3 lookfrom = Point3(0, 0, 0);  // Point camera is looking from
  Point3 lookat = Point3(0, 0, -1);  // Point camera is looking at
  Vec3 vup = Vec3(0, 1, 0);  // Camera-relative "up" direction

  private:
  int width = 600;  // NOLINT // Rendered Image Width
  int height = 400;  // NOLINT // Rendered Image Height
  int SamplesPerPixel = 100;  // NOLINT  // Count of random samples for each pixel
  double PixelSamplesScale = 1.0 / SamplesPerPixel;  // Color scale factor for a sum of pixel samples
  Vec3 u, v, w;  // Camera frame basis vectors

  HittableList World;
  std::vector<std::uint8_t> rlPixels;
  std::vector<Colour> pixels;

  [[nodiscard]] Colour RayColour(const Ray& ray, int depth, const class Hittable& World) const;

  public:
  [[nodiscard]] int GetSamplesPerPixel() const noexcept
  {
    return SamplesPerPixel;
  }
  void SetSamplesPerPixel(int spp)
  {
    SamplesPerPixel = spp;
    PixelSamplesScale = 1.0 / spp;
  }

  [[nodiscard]] HittableList& GetWorld() { return World; }
  void ResizeViewport(int width, int height);

  [[nodiscard]] Ray GetRayForPixel(int x, int y, const Vec3& pixel00_loc, const Vec3& pixel_delta_u, const Vec3& pixel_delta_v) const;
  [[nodiscard]] const std::vector<std::uint8_t>& GetRGBAData();
  void Render();
  [[nodiscard]] const std::vector<Colour>& GetPixelData() const { return pixels; }
};
