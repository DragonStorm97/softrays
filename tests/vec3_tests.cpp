#include "math.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>

using Catch::Matchers::WithinRel;
using softrays::Vec3;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

TEST_CASE("Vec3 basic construction and initialization")
{
  Vec3 v;
  REQUIRE_THAT(v.x, WithinRel(0.0));
  REQUIRE_THAT(v.y, WithinRel(0.0));
  REQUIRE_THAT(v.z, WithinRel(0.0));

  Vec3 v2{.x = 1.0, .y = 2.0, .z = 3.0};
  REQUIRE_THAT(v2.x, WithinRel(1.0));
  REQUIRE_THAT(v2.y, WithinRel(2.0));
  REQUIRE_THAT(v2.z, WithinRel(3.0));
}

TEST_CASE("Vec3 unary minus operator")
{
  Vec3 v{.x = 1.0, .y = -2.0, .z = 3.0};
  Vec3 neg = -v;
  REQUIRE_THAT(neg.x, WithinRel(-1.0));
  REQUIRE_THAT(neg.y, WithinRel(2.0));
  REQUIRE_THAT(neg.z, WithinRel(-3.0));
}

TEST_CASE("Vec3 addition and subtraction")
{
  Vec3 v1{.x = 1.0, .y = 2.0, .z = 3.0};
  Vec3 v2{.x = -1.0, .y = 4.0, .z = -3.0};
  Vec3 sum = v1 + v2;
  REQUIRE_THAT(sum.x, WithinRel(0.0));
  REQUIRE_THAT(sum.y, WithinRel(6.0));
  REQUIRE_THAT(sum.z, WithinRel(0.0));

  Vec3 diff = v1 - v2;
  REQUIRE_THAT(diff.x, WithinRel(2.0));
  REQUIRE_THAT(diff.y, WithinRel(-2.0));
  REQUIRE_THAT(diff.z, WithinRel(6.0));
}

TEST_CASE("Vec3 scalar multiplication and division")
{
  Vec3 v{.x = 1.0, .y = -2.0, .z = 3.0};
  v *= 2.0;
  REQUIRE_THAT(v.x, WithinRel(2.0));
  REQUIRE_THAT(v.y, WithinRel(-4.0));
  REQUIRE_THAT(v.z, WithinRel(6.0));

  v /= 2.0;
  REQUIRE_THAT(v.x, WithinRel(1.0));
  REQUIRE_THAT(v.y, WithinRel(-2.0));
  REQUIRE_THAT(v.z, WithinRel(3.0));
}

TEST_CASE("Vec3 Length and LengthSquared")
{
  Vec3 v{.x = 3.0, .y = 4.0, .z = 0.0};
  REQUIRE_THAT(v.LengthSquared(), WithinRel(25.0));
  REQUIRE_THAT(v.Length(), WithinRel(5.0));
}

TEST_CASE("Vec3 dot product")
{
  Vec3 v1{.x = 1.0, .y = 2.0, .z = 3.0};
  Vec3 v2{.x = 4.0, .y = -5.0, .z = 6.0};
  REQUIRE_THAT(v1.Dot(v2), WithinRel(12.0));
}

TEST_CASE("Vec3 cross product")
{
  Vec3 v1{.x = 1.0, .y = 2.0, .z = 3.0};
  Vec3 v2{.x = 4.0, .y = 5.0, .z = 6.0};
  Vec3 cross = v1.Cross(v2);
  REQUIRE_THAT(cross.x, WithinRel(-3.0));
  REQUIRE_THAT(cross.y, WithinRel(6.0));
  REQUIRE_THAT(cross.z, WithinRel(-3.0));
}

TEST_CASE("Vec3 UnitVector")
{
  Vec3 v{.x = 3.0, .y = 4.0, .z = 0.0};
  Vec3 unit = v.UnitVector();
  REQUIRE_THAT(unit.Length(), WithinRel(1.0));
  REQUIRE_THAT(unit.x, WithinRel(0.6));
  REQUIRE_THAT(unit.y, WithinRel(0.8));
}

TEST_CASE("Vec3 NearZero")
{
  Vec3 v1{.x = 1e-9, .y = 1e-9, .z = 1e-9};
  Vec3 v2{.x = 1.0, .y = 0.0, .z = 0.0};
  REQUIRE(v1.NearZero());
  REQUIRE_FALSE(v2.NearZero());
}

TEST_CASE("Vec3 Reflection")
{
  Vec3 v{.x = 1.0, .y = -1.0, .z = 0.0};
  Vec3 normal{.x = 0.0, .y = 1.0, .z = 0.0};
  Vec3 reflected = v.Reflect(normal);
  REQUIRE_THAT(reflected.x, WithinRel(1.0));
  REQUIRE_THAT(reflected.y, WithinRel(1.0));
  REQUIRE_THAT(reflected.z, WithinRel(0.0));
}

TEST_CASE("Vec3 Refraction")
{
  const Vec3 incident{.x = 1.0, .y = -1.0, .z = 0.0};
  const Vec3 normal{.x = 0.0, .y = 1.0, .z = 0.0};
  const double etai_over_etat = 0.5;  // Assume light entering from a denser to a rarer medium

  const Vec3 refracted = incident.Refract(normal, etai_over_etat);

  // Expected values (calculated manually or from reference)
  REQUIRE_THAT(refracted.x, WithinRel(0.5));
  REQUIRE_THAT(refracted.y, WithinRel(-0.8660254037844386));
  REQUIRE_THAT(refracted.z, WithinRel(0.0));
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
