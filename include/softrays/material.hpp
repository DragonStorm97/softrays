#pragma once

#include <raytracer.hpp>

class lambertian : public RayTracer::Material {
  public:
  lambertian(const Colour& albedo) : albedo(albedo) { }
  [[nodiscard]] bool Scatter(const RayTracer::Ray& r_in, const RayTracer::HitData& hit,
      Colour& attenuation, RayTracer::Ray& scattered) const override
  {
    auto scatter_direction = hit.Normal + Vec3::RandomUnitVector();

    // Catch degenerate scatter direction
    if (scatter_direction.near_zero())
      scatter_direction = hit.Normal;
    scattered = {hit.Location, scatter_direction};
    attenuation = albedo;
    return true;
  }

  Colour albedo{};
};

class metal : public RayTracer::Material {
  public:
  metal(const Colour& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) { }
  [[nodiscard]] bool Scatter(const RayTracer::Ray& r_in, const RayTracer::HitData& hit,
      Colour& attenuation, RayTracer::Ray& scattered) const override
  {
    Vec3 reflected = r_in.Direction.Reflect(hit.Normal);
    reflected = reflected.unit_vector() + (Vec3::RandomUnitVector() * fuzz);
    scattered = {hit.Location, reflected};
    attenuation = albedo;
    return scattered.Direction.dot(hit.Normal) > 0;
  }

  Colour albedo{};
  double fuzz{};
};
