#pragma once

#include "hittable.hpp"
#include "math.hpp"
#include "texture.hpp"
#include <memory>
#include <utility>

namespace softrays {
struct MaterialBase {
  MaterialBase() = default;
  MaterialBase(const MaterialBase&) = default;
  MaterialBase(MaterialBase&&) = delete;
  MaterialBase& operator=(const MaterialBase&) = default;
  MaterialBase& operator=(MaterialBase&&) = delete;

  virtual ~MaterialBase() = default;

  [[nodiscard]] virtual Colour emitted([[maybe_unused]] double u, [[maybe_unused]] double v, [[maybe_unused]] const Point3& loc) const noexcept
  {
    return {0, 0, 0};
  }

  [[nodiscard]] virtual bool Scatter([[maybe_unused]] const Ray& ray, [[maybe_unused]] const HitData& hit, [[maybe_unused]] Colour& attenuation, [[maybe_unused]] Ray& scattered) const
  {
    return false;
  }
};

class Lambertian : public MaterialBase {
  public:
  Lambertian(std::shared_ptr<Texture> tex) : texture(std::move(tex)) { }
  Lambertian(const Colour& albedo) : texture(std::make_shared<SolidTexture>(albedo)) { }
  [[nodiscard]] bool Scatter([[maybe_unused]] const Ray& r_in, const HitData& hit,
      Colour& attenuation, Ray& scattered) const override
  {
    auto scatter_direction = hit.Normal + Vec3::RandomUnitVector();

    // Catch degenerate scatter direction
    if (scatter_direction.NearZero())
      scatter_direction = hit.Normal;
    scattered = {.Origin = hit.Location, .Direction = scatter_direction};
    attenuation = texture->Value(hit.U, hit.V, hit.Location);
    return true;
  }
  std::shared_ptr<Texture> texture;
};

class Metal : public MaterialBase {
  public:
  Metal(const Colour& albedo, double fuzz) : Albedo(albedo), Fuzz(fuzz < 1 ? fuzz : 1) { }
  [[nodiscard]] bool Scatter(const Ray& r_in, const HitData& hit,
      Colour& attenuation, Ray& scattered) const override
  {
    Vec3 reflected = r_in.Direction.Reflect(hit.Normal);
    reflected = reflected.UnitVector() + (Vec3::RandomUnitVector() * Fuzz);
    scattered = {.Origin = hit.Location, .Direction = reflected};
    attenuation = Albedo;
    return scattered.Direction.Dot(hit.Normal) > 0;
  }

  Colour Albedo{};
  double Fuzz{};
};

class Dielectric : public MaterialBase {
  public:
  Dielectric(double refraction_index) noexcept : RefractionIndex(refraction_index) { }
  [[nodiscard]] bool Scatter(const Ray& r_in, const HitData& hit,
      Colour& attenuation, Ray& scattered) const override
  {
    attenuation = {.x = 1.0, .y = 1.0, .z = 1.0};
    double ri = hit.FrontFace ? (1.0 / RefractionIndex) : RefractionIndex;

    Vec3 unit_direction = r_in.Direction.UnitVector();
    double cos_theta = std::fmin((-unit_direction).Dot(hit.Normal), 1.0);
    double sin_theta = std::sqrt(1.0 - (cos_theta * cos_theta));

    bool cannot_refract = ri * sin_theta > 1.0;
    Vec3 direction = (cannot_refract || Reflectance(cos_theta, RefractionIndex) > RandomDouble()) ? unit_direction.Reflect(hit.Normal) : unit_direction.Refract(hit.Normal, ri);

    scattered = {.Origin = hit.Location, .Direction = direction};

    return true;
  }

  // Refractive index in vacuum or air, or the ratio of the material's refractive index over
  // the refractive index of the enclosing media
  double RefractionIndex;

  constexpr static double Reflectance(double cosine, double refraction_index) noexcept
  {
    // Use Schlick's approximation for reflectance.
    auto r0 = (1 - refraction_index) / (1 + refraction_index);
    r0 = r0 * r0;
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    return r0 + ((1 - r0) * std::pow((1 - cosine), 5));
  }
};

class DiffuseLight : public MaterialBase {
  public:
  DiffuseLight(double scale, std::shared_ptr<Texture> tex) : emission_factor(scale), texture(std::move(tex)) { }
  DiffuseLight(std::shared_ptr<Texture> tex) : texture(std::move(tex)) { }
  DiffuseLight(const Colour& emit) : texture(std::make_shared<SolidTexture>(emit)) { }

  [[nodiscard]] Colour emitted(double u, double v, const Point3& loc) const noexcept override
  {
    return texture->Value(u, v, loc) * emission_factor;
  }

  private:
  double emission_factor{1.0};
  std::shared_ptr<Texture> texture;
};

class Isotropic : public MaterialBase {
  public:
  Isotropic(const Colour& albedo) : texture(std::make_shared<SolidTexture>(albedo)) { }
  Isotropic(std::shared_ptr<Texture> tex) : texture(std::move(tex)) { }

  [[nodiscard]] bool Scatter([[maybe_unused]] const Ray& ray, const HitData& hit,
      Colour& attenuation, Ray& scattered) const override
  {
    scattered = Ray(hit.Location, Vec3::RandomUnitVector());
    attenuation = texture->Value(hit.U, hit.V, hit.Location);
    return true;
  }

  private:
  std::shared_ptr<Texture> texture;
};
}
