#include "material.hpp"
#include "raytracer.hpp"
#include "shapes.hpp"
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace softrays;
// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
TEST_CASE("softrays Bencmarking")
{
  BENCHMARK("Simple Scene")
  {
    RayTracer raytracer;
    raytracer.ResizeViewport({.Width = 100, .Height = 100});
    raytracer.SetSamplesPerPixel(5);
    raytracer.MaxDepth = 5;
    raytracer.FieldOfView = 90;
    raytracer.LookFrom = Point3(13, 2, 3);
    raytracer.LookAt = Point3(0, 0, 0);
    raytracer.CameraUp = Vec3(0, 1, 0);
    raytracer.DefocusAngle = 0.6;
    raytracer.FocusDistance = 10.0;

    // setup the scene:
    auto& world = raytracer.GetWorld();
    auto ground_material = std::make_shared<Lambertian>(Colour{0.5, 0.5, 0.5});  // NOLINT
    auto sphere_mat1 = std::make_shared<Metal>(Colour{0.5, 0.5, 0.5}, 0.5);  // NOLINT
    auto sphere_mat2 = std::make_shared<Dielectric>(0.55);  // NOLINT
    world.Add(make_shared<Sphere>(Point3(10, 0, 0), 0, ground_material));
    world.Add(make_shared<Sphere>(Point3(0, 0, 10), 0, sphere_mat1));
    world.Add(make_shared<Sphere>(Point3(0, -1000, 0), 0, sphere_mat2));
    // Render
    raytracer.Render();
    return raytracer.GetPixelData();
  };
}
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
