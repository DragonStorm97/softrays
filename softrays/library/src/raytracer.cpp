#include "raytracer.hpp"
#include "material.hpp"  //NOLINT(unused-includes) for implementation of MaterialBase
#include "math.hpp"
#include "utility.hpp"

#include <cstddef>

using namespace softrays;

Point3 RayTracer::DefocusDiskSample() const noexcept
{
  // Returns a random point in the camera defocus disk.
  const auto rand = Vec3::RandomInUnitDisk();
  return CameraPosition + (DefocusDisk_u * rand.x) + (DefocusDisk_v * rand.y);
}

Ray RayTracer::GetRayForPixel(int x, int y, const Vec3& pixel00_loc, const Vec3& pixel_delta_u, const Vec3& pixel_delta_v) const
{
  // Construct a camera ray originating from the defocus disk and directed at a randomly
  // point around the pixel location x, y.

  const auto offset = RandomInUnitSquare();
  const auto pixel_sample = pixel00_loc
      + (pixel_delta_u * (x + offset.x))
      + (pixel_delta_v * (y + offset.y));

  const auto ray_origin = (DefocusAngle <= 0) ? CameraPosition : DefocusDiskSample();
  const auto ray_direction = pixel_sample - ray_origin;

  return Ray{.Origin = ray_origin, .Direction = ray_direction};
}

void RayTracer::SetupCamera()
{
  CameraPosition = LookFrom;

  // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
  Camera_w = (LookFrom - LookAt).UnitVector();
  Camera_u = CameraUp.Cross(Camera_w).UnitVector();
  Camera_v = Camera_w.Cross(Camera_u);
}

void RayTracer::Render()
{
  SetupCamera();

  const auto theta = DegreesToRadians(FieldOfView);
  const auto hyp = std::tan(theta / 2);
  const auto viewport_height = 2 * hyp * FocusDistance;
  const auto viewport_width = viewport_height * (static_cast<double>(ViewportDimensions.Width) / ViewportDimensions.Height);

  // Calculate the vectors across the horizontal and down the vertical viewport edges.

  const Vec3 viewport_u = Camera_u * viewport_width;  // Vector across viewport horizontal edge
  const Vec3 viewport_v = (-Camera_v) * viewport_height;  // Vector down viewport vertical edge

  // Calculate the horizontal and vertical delta vectors from pixel to pixel.
  const auto pixel_delta_u = viewport_u / ViewportDimensions.Width;
  const auto pixel_delta_v = viewport_v / ViewportDimensions.Height;

  // Calculate the location of the upper left pixel.
  const auto viewport_upper_left = CameraPosition - (Camera_w * FocusDistance) - (viewport_u / 2) - (viewport_v / 2);
  const auto pixel00_loc = viewport_upper_left + ((pixel_delta_u + pixel_delta_v) * 0.5);

  // Calculate the camera defocus disk basis vectors.
  const auto defocus_radius = FocusDistance * std::tan(DegreesToRadians(DefocusAngle / 2));

  DefocusDisk_u = Camera_u * defocus_radius;
  DefocusDisk_v = Camera_v * defocus_radius;

  for (int y = 0; y < ViewportDimensions.Height; ++y) {
    for (int x = 0; x < ViewportDimensions.Width; ++x) {
      Colour pixel_colour{};
      if (SamplesPerPixel > 1) {
        for (int sample = 0; sample < SamplesPerPixel; ++sample) {
          const auto ray = GetRayForPixel(x, y, pixel00_loc, pixel_delta_u, pixel_delta_v);
          pixel_colour += RayColour(ray, MaxDepth, World);
        }
      } else {
        const auto pixel_center = pixel00_loc + (pixel_delta_u * x) + (pixel_delta_v * y);
        const auto ray_direction = pixel_center - CameraPosition;
        const Ray ray(CameraPosition, ray_direction);
        pixel_colour = RayColour(ray, MaxDepth, World);
      }

      const auto pixel_start = static_cast<std::size_t>(y * ViewportDimensions.Width) + static_cast<std::size_t>(x);
      PixelData[pixel_start] = pixel_colour * PixelSamplesScale;
    }
  }
}

// TODO: we have a maxDepth, so we could remove this recursion in favour of a static array and loop
Colour RayTracer::RayColour(const Ray& ray, int depth, const Hittable& world) const
{
  // If we've exceeded the ray bounce limit, no more light is gathered.
  if (depth <= 0) {
    return Colour{0, 0, 0};
  }

  HitData hit;
  constexpr auto minDist = 0.001;
  if (world.Hit(ray, {.Min = minDist, .Max = Infinity}, hit)) {
    Ray scattered{};
    Colour attenuation{};
    if (hit.Material->Scatter(ray, hit, attenuation, scattered)) {
      return RayColour(scattered, depth - 1, World) * attenuation;
    }
    return {0, 0, 0};
  }

  Vec3 unit_direction = ray.Direction.UnitVector();
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
  auto a = 0.5 * (unit_direction.y + 1.0);
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
  return (Colour{1.0, 1.0, 1.0} * (1.0 - a)) + (Colour{0.5, 0.7, 1.0} * a);
}

void RayTracer::ResizeViewport(const Dimension2d& dim)
{
  ViewportDimensions = dim;
  PixelData.clear();
  PixelData.resize(static_cast<std::size_t>(ViewportDimensions.Width) * static_cast<std::size_t>(ViewportDimensions.Height), {0.0, 0.0, 0.0});
  rlPixels.clear();
  rlPixels.resize(static_cast<std::size_t>(ViewportDimensions.Width) * static_cast<std::size_t>(ViewportDimensions.Height) * 4UL, 0);
}

const std::vector<std::uint8_t>& RayTracer::GetRGBAData()
{
  static constexpr Interval intensity(0.000, 0.999);
  static constexpr int byteMax{256};
  for (int y = 0; y < ViewportDimensions.Height; ++y) {
    for (int x = 0; x < ViewportDimensions.Width; ++x) {
      const auto pixel_start = static_cast<std::size_t>(y * ViewportDimensions.Width) + static_cast<std::size_t>(x);
      const auto rl_pixel_start = static_cast<std::size_t>((y * ViewportDimensions.Width) + x) * 4UL;

      // NOTE: are we supposed to do gamma-correction here? (it does look more like the book with it)
      // const auto r = (pixels[pixel_start].x);
      // const auto g = (pixels[pixel_start].y);
      // const auto b = (pixels[pixel_start].z);

      const auto r = LinearToGamma(PixelData[pixel_start].x);
      const auto g = LinearToGamma(PixelData[pixel_start].y);
      const auto b = LinearToGamma(PixelData[pixel_start].z);

      rlPixels[rl_pixel_start] = static_cast<std::uint8_t>(intensity.Clamp(r) * byteMax);
      rlPixels[rl_pixel_start + 1] = static_cast<std::uint8_t>(intensity.Clamp(g) * byteMax);
      rlPixels[rl_pixel_start + 2] = static_cast<std::uint8_t>(intensity.Clamp(b) * byteMax);
      rlPixels[rl_pixel_start + 3] = byteMax - 1;
    }
  }
  return rlPixels;
}
