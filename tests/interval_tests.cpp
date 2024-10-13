#include "math.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>

using Catch::Matchers::WithinRel;
using softrays::Interval;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
TEST_CASE("Interval basic functionality")
{
  const Interval interval{.Min = -10.0, .Max = 10.0};

  // Test Size
  REQUIRE_THAT(interval.Size(), WithinRel(20.0));

  // Test Contains
  REQUIRE(interval.Contains(0.0));  // Should be inside the interval
  REQUIRE(interval.Contains(-10.0));  // Boundary case
  REQUIRE(interval.Contains(10.0));  // Boundary case
  REQUIRE_FALSE(interval.Contains(11.0));  // Outside the interval

  // Test Surrounds
  REQUIRE(interval.Surrounds(0.0));  // Inside, but not on the boundary
  REQUIRE_FALSE(interval.Surrounds(-10.0));  // Boundary case
  REQUIRE_FALSE(interval.Surrounds(10.0));  // Boundary case

  // Test Clamp
  REQUIRE_THAT(interval.Clamp(5.0), WithinRel(5.0));  // Inside the interval
  REQUIRE_THAT(interval.Clamp(-15.0), WithinRel(-10.0));  // Below the Min
  REQUIRE_THAT(interval.Clamp(15.0), WithinRel(10.0));  // Above the Max
}

TEST_CASE("Empty and Universe intervals")
{
  // Test the Empty interval
  REQUIRE_THAT(Interval::Empty.Size(), WithinRel(-softrays::Infinity));  // Empty interval should have size -Infinity
  REQUIRE_FALSE(Interval::Empty.Contains(0.0));  // Empty should contain no points
  REQUIRE_FALSE(Interval::Empty.Surrounds(0.0));

  // Test the Universe interval
  REQUIRE_THAT(Interval::Universe.Size(), WithinRel(softrays::Infinity));  // Universe interval is infinite
  REQUIRE(Interval::Universe.Contains(0.0));  // Universe contains everything
  REQUIRE(Interval::Universe.Surrounds(0.0));
  REQUIRE_THAT(Interval::Universe.Clamp(100.0), WithinRel(100.0));  // Universe clamping does nothing
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
