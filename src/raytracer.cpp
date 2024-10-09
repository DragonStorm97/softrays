#include "shapes.hpp"
#include <raytracer.hpp>

Vec3 sample_square()
{
  // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
  return Vec3(RandomDouble() - 0.5, RandomDouble() - 0.5, 0);
}

RayTracer::Ray RayTracer::GetRayForPixel(int x, int y, const Vec3& pixel00_loc, const Vec3& pixel_delta_u, const Vec3& pixel_delta_v) const
{
  // Construct a camera ray originating from the origin and directed at randomly sampled
  // point around the pixel location x, y.

  auto offset = sample_square();
  auto pixel_sample = pixel00_loc
      + (pixel_delta_u * (x + offset.x))
      + (pixel_delta_v * (y + offset.y));

  auto ray_origin = CameraPosition;
  auto ray_direction = pixel_sample - ray_origin;

  return Ray{ray_origin, ray_direction};
}

void RayTracer::Render()
{
  const auto focal_length = 1.0;
  const auto viewport_height = 2.0;  // NOLINT
  const auto viewport_width = viewport_height * (static_cast<double>(width) / height);

  // Calculate the vectors across the horizontal and down the vertical viewport edges.
  const auto viewport_u = Vec3(viewport_width, 0, 0);
  const auto viewport_v = Vec3(0, -viewport_height, 0);

  // Calculate the horizontal and vertical delta vectors from pixel to pixel.
  const auto pixel_delta_u = viewport_u / width;
  const auto pixel_delta_v = viewport_v / height;

  // Calculate the location of the upper left pixel.
  const auto viewport_upper_left = CameraPosition
      - Vec3(0, 0, focal_length) - viewport_u / 2.0 - viewport_v / 2.0;
  const auto pixel00_loc = viewport_upper_left + ((pixel_delta_u + pixel_delta_v) * 0.5);

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      Colour pixel_colour{};
      if (SamplesPerPixel > 1) {
        for (int sample = 0; sample < SamplesPerPixel; ++sample) {
          const auto ray = GetRayForPixel(x, y, pixel00_loc, pixel_delta_u, pixel_delta_v);
          pixel_colour += RayColour(ray, World);
        }
      } else {
        const auto pixel_center = pixel00_loc + (pixel_delta_u * x) + (pixel_delta_v * y);
        const auto ray_direction = pixel_center - CameraPosition;
        const Ray ray(CameraPosition, ray_direction);
        pixel_colour = RayColour(ray, World);
      }

      const auto pixel_start = static_cast<std::size_t>((y * width) + x);
      pixels[pixel_start] = pixel_colour * PixelSamplesScale;
    }
  }
}

Colour RayTracer::RayColour(const Ray& ray, const Hittable& world) const
{
  HitData hit;
  if (world.Hit(ray, {0, Infinity}, hit)) {
    return (hit.Normal + Colour{1, 1, 1}) * 0.5;
  }

  Vec3 unit_direction = ray.Direction.unit_vector();
  auto a = 0.5 * (unit_direction.y + 1.0);
  return (Colour{1.0, 1.0, 1.0} * (1.0 - a)) + (Colour{0.5, 0.7, 1.0} * a);
}

void RayTracer::ResizeViewport(int Width, int Height)
{
  this->width = Width;
  this->height = Height;
  pixels.clear();
  pixels.resize(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), {0.0, 0.0, 0.0});
  rlPixels.clear();
  rlPixels.resize(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4UL, 90);
}

const std::vector<std::uint8_t>& RayTracer::GetRGBAData()
{
  static constexpr Interval intensity(0.000, 0.999);
  static constexpr int byteMax{256};
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      const auto pixel_start = static_cast<std::size_t>((y * width) + x);
      const auto rl_pixel_start = static_cast<std::size_t>(((y * width) + x) * 4);
      rlPixels[rl_pixel_start] = static_cast<std::uint8_t>(intensity.Clamp(pixels[pixel_start].x) * byteMax);
      rlPixels[rl_pixel_start + 1] = static_cast<std::uint8_t>(intensity.Clamp(pixels[pixel_start].y) * byteMax);
      rlPixels[rl_pixel_start + 2] = static_cast<std::uint8_t>(intensity.Clamp(pixels[pixel_start].z) * byteMax);
      rlPixels[rl_pixel_start + 3] = 255;
    }
  }
  return rlPixels;
}
