#pragma once

#include "utility.hpp"
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

class dielectric : public RayTracer::Material {
  public:
  dielectric(double refraction_index) noexcept : refraction_index(refraction_index) { }
  [[nodiscard]] bool Scatter(const RayTracer::Ray& r_in, const RayTracer::HitData& hit,
      Colour& attenuation, RayTracer::Ray& scattered) const override
  {
    attenuation = {1.0, 1.0, 1.0};
    double ri = hit.FrontFace ? (1.0 / refraction_index) : refraction_index;

    Vec3 unit_direction = r_in.Direction.unit_vector();
    double cos_theta = std::fmin((-unit_direction).dot(hit.Normal), 1.0);
    double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

    bool cannot_refract = ri * sin_theta > 1.0;
    Vec3 direction = (cannot_refract || Reflectance(cos_theta, refraction_index) > RandomDouble()) ? unit_direction.Reflect(hit.Normal) : unit_direction.Refract(hit.Normal, ri);

    scattered = RayTracer::Ray(hit.Location, direction);

    return true;
  }

  // Refractive index in vacuum or air, or the ratio of the material's refractive index over
  // the refractive index of the enclosing media
  double refraction_index;

  static double Reflectance(double cosine, double refraction_index)
  {
    // Use Schlick's approximation for reflectance.
    auto r0 = (1 - refraction_index) / (1 + refraction_index);
    r0 = r0 * r0;
    return r0 + (1 - r0) * std::pow((1 - cosine), 5);
  }
};
