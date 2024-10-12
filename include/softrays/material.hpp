#pragma once

#include "math.hpp"
#include <raytracer.hpp>

class Lambertian : public RayTracer::Material {
  public:
  Lambertian(const Colour& albedo) : Albedo(albedo) { }
  [[nodiscard]] bool Scatter([[maybe_unused]] const RayTracer::Ray& r_in, const RayTracer::HitData& hit,
      Colour& attenuation, RayTracer::Ray& scattered) const override
  {
    auto scatter_direction = hit.Normal + Vec3::RandomUnitVector();

    // Catch degenerate scatter direction
    if (scatter_direction.near_zero())
      scatter_direction = hit.Normal;
    scattered = {.Origin = hit.Location, .Direction = scatter_direction};
    attenuation = Albedo;
    return true;
  }

  Colour Albedo{};
};

class Metal : public RayTracer::Material {
  public:
  Metal(const Colour& albedo, double fuzz) : Albedo(albedo), Fuzz(fuzz < 1 ? fuzz : 1) { }
  [[nodiscard]] bool Scatter(const RayTracer::Ray& r_in, const RayTracer::HitData& hit,
      Colour& attenuation, RayTracer::Ray& scattered) const override
  {
    Vec3 reflected = r_in.Direction.Reflect(hit.Normal);
    reflected = reflected.unit_vector() + (Vec3::RandomUnitVector() * Fuzz);
    scattered = {.Origin = hit.Location, .Direction = reflected};
    attenuation = Albedo;
    return scattered.Direction.dot(hit.Normal) > 0;
  }

  Colour Albedo{};
  double Fuzz{};
};

class Dielectric : public RayTracer::Material {
  public:
  Dielectric(double refraction_index) noexcept : RefractionIndex(refraction_index) { }
  [[nodiscard]] bool Scatter(const RayTracer::Ray& r_in, const RayTracer::HitData& hit,
      Colour& attenuation, RayTracer::Ray& scattered) const override
  {
    attenuation = {.x = 1.0, .y = 1.0, .z = 1.0};
    double ri = hit.FrontFace ? (1.0 / RefractionIndex) : RefractionIndex;

    Vec3 unit_direction = r_in.Direction.unit_vector();
    double cos_theta = std::fmin((-unit_direction).dot(hit.Normal), 1.0);
    double sin_theta = std::sqrt(1.0 - (cos_theta * cos_theta));

    bool cannot_refract = ri * sin_theta > 1.0;
    Vec3 direction = (cannot_refract || Reflectance(cos_theta, RefractionIndex) > RandomDouble()) ? unit_direction.Reflect(hit.Normal) : unit_direction.Refract(hit.Normal, ri);

    scattered = RayTracer::Ray(hit.Location, direction);

    return true;
  }

  // Refractive index in vacuum or air, or the ratio of the material's refractive index over
  // the refractive index of the enclosing media
  double RefractionIndex;

  static double Reflectance(double cosine, double refraction_index)
  {
    // Use Schlick's approximation for reflectance.
    auto r0 = (1 - refraction_index) / (1 + refraction_index);
    r0 = r0 * r0;
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    return r0 + ((1 - r0) * std::pow((1 - cosine), 5));
  }
};
