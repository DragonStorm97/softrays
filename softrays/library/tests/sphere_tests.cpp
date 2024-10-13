
#include "math.hpp"
#include "shapes.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <memory>

using Catch::Matchers::WithinRel;
using softrays::HitData;
using softrays::Interval;
using softrays::MaterialBase;
using softrays::Point3;
using softrays::Ray;
using softrays::Sphere;
using softrays::Vec3;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
TEST_CASE("Sphere ray intersection tests")
{
  // Define a simple material (we can mock it or pass a nullptr if it's not used in the hit logic)
  auto material = std::make_shared<MaterialBase>();

  // Define a sphere with center at (0, 0, 0) and radius 1
  Sphere sphere(Point3(0.0, 0.0, 0.0), 1.0, std::move(material));

  // Test ray that hits the sphere
  SECTION("Ray hits the sphere")
  {
    Ray ray(Point3(0.0, 0.0, -3.0), Vec3(0.0, 0.0, 1.0));  // Ray starting outside, heading toward the sphere
    Interval interval(0.001, softrays::Infinity);  // Valid ray time interval
    HitData hit_data;

    REQUIRE(sphere.Hit(ray, interval, hit_data));

    // Check hit data
    REQUIRE_THAT(hit_data.Time, WithinRel(2.0));  // Expect hit time to be 2
    REQUIRE((hit_data.Location - Point3(0.0, 0.0, -1.0)).NearZero());  // Expect hit location on the sphere surface
    REQUIRE((hit_data.Normal - Point3(0.0, 0.0, -1.0)).NearZero());  // Expect outward normal at the hit point
  }

  // Test ray that misses the sphere
  SECTION("Ray misses the sphere")
  {
    Ray ray(Point3(0.0, 0.0, -3.0), Vec3(1.0, 0.0, 0.0));  // Ray heading in the x-direction, missing the sphere
    Interval interval(0.001, softrays::Infinity);
    HitData hit_data;

    REQUIRE_FALSE(sphere.Hit(ray, interval, hit_data));  // Should miss the sphere
  }

  // Test ray that starts inside the sphere
  SECTION("Ray starts inside the sphere")
  {
    Ray ray(Point3(0.0, 0.0, 0.5), Vec3(0.0, 0.0, 1.0));  // Ray starting inside the sphere
    Interval interval(0.001, softrays::Infinity);
    HitData hit_data;

    REQUIRE(sphere.Hit(ray, interval, hit_data));

    // Check hit data for exiting the sphere
    REQUIRE_THAT(hit_data.Time, WithinRel(0.5));  // Expect hit time to be 0.5
    REQUIRE((hit_data.Location - Point3(0.0, 0.0, 1.0)).NearZero());  // Exiting through the positive z surface
                                                                      // Normal pointing outward
    REQUIRE_THAT(hit_data.Normal.x, WithinRel(0.0));
    REQUIRE_THAT(hit_data.Normal.y, WithinRel(0.0));
    REQUIRE_THAT(hit_data.Normal.z, WithinRel(-1.0));
  }

  // Test ray that tangentially touches the sphere
  SECTION("Ray tangentially touches the sphere")
  {
    Ray ray(Point3(-1.0, 0.0, -3.0), Vec3(0.0, 0.0, 1.0));  // Ray moving parallel to x-axis, tangent to the sphere
    Interval interval(0.001, softrays::Infinity);
    HitData hit_data;

    REQUIRE(sphere.Hit(ray, interval, hit_data));

    // Check hit data for tangent hit
    REQUIRE_THAT(hit_data.Time, WithinRel(3.0));  // Expect hit time to be 3
    REQUIRE((hit_data.Location - Point3(-1.0, 0.0, 0.0)).NearZero());  // Touches the sphere at (-1, 0, 0)
    // Normal should be pointing outward along the x-axis:
    REQUIRE_THAT(hit_data.Normal.x, WithinRel(1.0));
    REQUIRE_THAT(hit_data.Normal.y, WithinRel(0.0));
    REQUIRE_THAT(hit_data.Normal.z, WithinRel(0.0));
  }
}
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
