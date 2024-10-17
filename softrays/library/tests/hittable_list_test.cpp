#include "material.hpp"
#include "math.hpp"
#include "shapes.hpp"
#include "utility.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <memory>

using Catch::Matchers::WithinRel;
using softrays::Colour;
using softrays::HitData;
using softrays::HittableList;
using softrays::Interval;
using softrays::Lambertian;
using softrays::Point3;
using softrays::Sphere;
using softrays::Vec3;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
TEST_CASE("HittableList Hit function test")
{
  HittableList hittable_list;

  // Add two spheres to the list
  auto material = std::make_shared<Lambertian>(Colour(0.8, 0.3, 0.3));
  auto sphere1 = std::make_shared<Sphere>(Point3(0, 0, -1), 0.5, material);  // Sphere 1
  auto sphere2 = std::make_shared<Sphere>(Point3(0, -100.5, -1), 100, material);  // Sphere 2 (ground)

  hittable_list.Add(std::move(sphere1));
  hittable_list.Add(std::move(sphere2));

  // Define a ray that should hit sphere1
  softrays::Ray ray(Point3(0, 0, 0), Vec3(0, 0, -1));

  HitData hit_data;
  Interval ray_time{.Min = 0.01, .Max = softrays::Infinity};

  // Test hitting the spheres in the list
  bool hit = hittable_list.Hit(ray, ray_time, hit_data);

  // Ensure the ray hits something
  REQUIRE(hit == true);

  // Check that the hit occurred at the correct location (sphere1 at z = -1)
  REQUIRE_THAT(hit_data.Location.x, WithinRel(0.0));
  REQUIRE_THAT(hit_data.Location.y, WithinRel(0.0));
  REQUIRE_THAT(hit_data.Location.z, WithinRel(-0.5));

  // Ensure the hit was on sphere1, not sphere2 (closer object should be hit)
  REQUIRE_THAT(hit_data.Time, WithinRel(0.5));  // Sphere1 hit occurs at t = 0.5

  // Check that the normal at the hit point is correct
  REQUIRE_THAT(hit_data.Normal.x, WithinRel(0.0));
  REQUIRE_THAT(hit_data.Normal.y, WithinRel(0.0));
  REQUIRE_THAT(hit_data.Normal.z, WithinRel(1.0));
}

TEST_CASE("HittableList Miss function test")
{
  HittableList hittable_list;

  // Add a single sphere to the list
  auto material = std::make_shared<Lambertian>(Colour(0.8, 0.3, 0.3));
  auto sphere = std::make_shared<Sphere>(Point3(0, 0, -1), 0.5, std::move(material));
  hittable_list.Add(std::move(sphere));

  // Define a ray that misses the sphere
  softrays::Ray ray(Point3(0, 0, 0), Vec3(0, 1, 0));  // softrays::Ray in +y direction (away from sphere)

  HitData hit_data;
  Interval ray_time{.Min = 0.0, .Max = softrays::Infinity};

  // Test that the ray misses the sphere
  bool hit = hittable_list.Hit(ray, ray_time, hit_data);

  // Ensure the ray misses
  REQUIRE(hit == false);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
