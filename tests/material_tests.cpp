#include "material.hpp"
#include "math.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>

using Catch::Matchers::WithinRel;
using softrays::Colour;
using softrays::Dielectric;
using softrays::HitData;
using softrays::Lambertian;
using softrays::Metal;
using softrays::Point3;
using softrays::Ray;
using softrays::Vec3;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
TEST_CASE("Lambertian scatter test")
{
  Colour albedo(0.8, 0.3, 0.3);  // Define an albedo (color)
  Lambertian lambertian(albedo);

  Ray ray(Point3(0, 0, 0), Vec3(0, 0, 1));  // Incident ray
  HitData hit_data;
  hit_data.Location = Point3(0, 0, 1);
  hit_data.Normal = Vec3(0, 0, 1);

  Colour attenuation;
  Ray scattered;

  // Test scattering
  REQUIRE(lambertian.Scatter(ray, hit_data, attenuation, scattered));

  // Check the attenuation (should match the albedo)
  REQUIRE_THAT(attenuation.x, WithinRel(albedo.x));
  REQUIRE_THAT(attenuation.y, WithinRel(albedo.y));
  REQUIRE_THAT(attenuation.z, WithinRel(albedo.z));

  // Check that the scattered ray origin is correct
  REQUIRE_THAT(scattered.Origin.x, WithinRel(hit_data.Location.x));
  REQUIRE_THAT(scattered.Origin.y, WithinRel(hit_data.Location.y));
  REQUIRE_THAT(scattered.Origin.z, WithinRel(hit_data.Location.z));

  // Ensure the scattered direction is not a zero vector
  REQUIRE_FALSE(scattered.Direction.NearZero());
}

TEST_CASE("Metal scatter test")
{
  Colour albedo(0.8, 0.6, 0.2);  // Define an albedo (color)
  double fuzz = 0.3;
  Metal metal(albedo, fuzz);

  Ray ray(Point3(0, 0, 0), Vec3(0, 0, 1));  // Incident ray
  HitData hit_data;
  hit_data.Location = Point3(0, 0, 1);
  hit_data.Normal = Vec3(0, 0, 1);

  Colour attenuation;
  Ray scattered;

  // Test scattering
  [[maybe_unused]] auto was_scattered = (metal.Scatter(ray, hit_data, attenuation, scattered));

  // Check the attenuation (should match the albedo)
  REQUIRE_THAT(attenuation.x, WithinRel(albedo.x));
  REQUIRE_THAT(attenuation.y, WithinRel(albedo.y));
  REQUIRE_THAT(attenuation.z, WithinRel(albedo.z));

  // Check that the scattered ray origin is correct
  REQUIRE_THAT(scattered.Origin.x, WithinRel(hit_data.Location.x));
  REQUIRE_THAT(scattered.Origin.y, WithinRel(hit_data.Location.y));
  REQUIRE_THAT(scattered.Origin.z, WithinRel(hit_data.Location.z));

  // Ensure that the scattered ray is "reflected" (should go in a valid direction)
  REQUIRE(scattered.Direction.Dot(hit_data.Normal) < 0);
}

TEST_CASE("Dielectric scatter test - refraction")
{
  double refraction_index = 1.5;  // Glass-like material
  Dielectric dielectric(refraction_index);

  Ray ray(Point3(0, 0, 0), Vec3(0, 0, 1));  // Incident ray
  HitData hit_data;
  hit_data.Location = Point3(0, 0, 1);
  hit_data.Normal = Vec3(0, 0, 1);
  hit_data.FrontFace = true;  // Ray is coming from outside (air) into the dielectric

  Colour attenuation;
  Ray scattered;

  // Test scattering (should refract)
  REQUIRE(dielectric.Scatter(ray, hit_data, attenuation, scattered));

  // Check attenuation (dielectrics return white light)
  REQUIRE_THAT(attenuation.x, WithinRel(1.0));
  REQUIRE_THAT(attenuation.y, WithinRel(1.0));
  REQUIRE_THAT(attenuation.z, WithinRel(1.0));

  // Check that the scattered ray origin is correct
  REQUIRE_THAT(scattered.Origin.x, WithinRel(hit_data.Location.x));
  REQUIRE_THAT(scattered.Origin.y, WithinRel(hit_data.Location.y));
  REQUIRE_THAT(scattered.Origin.z, WithinRel(hit_data.Location.z));
}

TEST_CASE("Dielectric scatter test - total internal reflection")
{
  double refraction_index = 1.5;  // Glass-like material
  Dielectric dielectric(refraction_index);

  // Ray coming from inside the material (refraction will fail, should reflect)
  Ray ray(Point3(0, 0, 0), Vec3(0, 0, -1));  // Incident ray
  HitData hit_data;
  hit_data.Location = Point3(0, 0, 1);
  hit_data.Normal = Vec3(0, 0, 1);
  hit_data.FrontFace = false;  // Ray is coming from inside the dielectric

  Colour attenuation;
  Ray scattered;

  // Test scattering (should reflect due to total internal reflection)
  REQUIRE(dielectric.Scatter(ray, hit_data, attenuation, scattered));

  // Check attenuation (dielectrics return white light)
  REQUIRE_THAT(attenuation.x, WithinRel(1.0));
  REQUIRE_THAT(attenuation.y, WithinRel(1.0));
  REQUIRE_THAT(attenuation.z, WithinRel(1.0));

  // Check that the scattered ray origin is correct
  REQUIRE_THAT(scattered.Origin.x, WithinRel(hit_data.Location.x));
  REQUIRE_THAT(scattered.Origin.y, WithinRel(hit_data.Location.y));
  REQUIRE_THAT(scattered.Origin.z, WithinRel(hit_data.Location.z));

  // Check that the ray is reflected (not refracted)
  REQUIRE(scattered.Direction.Dot(hit_data.Normal) < 0);
}
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
