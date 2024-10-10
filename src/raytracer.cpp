#include "shapes.hpp"
#include "utility.hpp"
#include <raytracer.hpp>

Vec3 sample_square()
{
  // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
  return Vec3(RandomDouble() - 0.5, RandomDouble() - 0.5, 0);
}
Point3 RayTracer::DefocusDiskSample() const noexcept
{
  // Returns a random point in the camera defocus disk.
  const auto p = RandomInUnitDisk();
  return CameraPosition + (defocus_disk_u * p.x) + (defocus_disk_v * p.y);
}

RayTracer::Ray RayTracer::GetRayForPixel(int x, int y, const Vec3& pixel00_loc, const Vec3& pixel_delta_u, const Vec3& pixel_delta_v) const
{
  // Construct a camera ray originating from the defocus disk and directed at a randomly
  // point around the pixel location x, y.

  const auto offset = sample_square();
  const auto pixel_sample = pixel00_loc
      + (pixel_delta_u * (x + offset.x))
      + (pixel_delta_v * (y + offset.y));

  const auto ray_origin = (defocus_angle <= 0) ? CameraPosition : DefocusDiskSample();
  const auto ray_direction = pixel_sample - ray_origin;

  return Ray{ray_origin, ray_direction};
}

void RayTracer::Render()
{
  CameraPosition = lookfrom;

  const auto theta = DegreesToRadians(vfov);
  const auto h = std::tan(theta / 2);
  const auto viewport_height = 2 * h * focus_dist;
  const auto viewport_width = viewport_height * (static_cast<double>(width) / height);

  // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
  w = (lookfrom - lookat).unit_vector();
  u = vup.cross(w).unit_vector();
  v = w.cross(u);

  // Calculate the vectors across the horizontal and down the vertical viewport edges.

  const Vec3 viewport_u = u * viewport_width;  // Vector across viewport horizontal edge
  const Vec3 viewport_v = (-v) * viewport_height;  // Vector down viewport vertical edge

  // Calculate the horizontal and vertical delta vectors from pixel to pixel.
  const auto pixel_delta_u = viewport_u / width;
  const auto pixel_delta_v = viewport_v / height;

  // Calculate the location of the upper left pixel.
  const auto viewport_upper_left = CameraPosition - (w * focus_dist) - (viewport_u / 2) - (viewport_v / 2);
  const auto pixel00_loc = viewport_upper_left + ((pixel_delta_u + pixel_delta_v) * 0.5);

  // Calculate the camera defocus disk basis vectors.
  const auto defocus_radius = focus_dist * std::tan(DegreesToRadians(defocus_angle / 2));
  defocus_disk_u = u * defocus_radius;
  defocus_disk_v = v * defocus_radius;

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
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

      const auto pixel_start = static_cast<std::size_t>((y * width) + x);
      pixels[pixel_start] = pixel_colour * PixelSamplesScale;
    }
  }
}

Colour RayTracer::RayColour(const Ray& ray, int depth, const Hittable& world) const
{
  // If we've exceeded the ray bounce limit, no more light is gathered.
  if (depth <= 0) {
    return Colour{0, 0, 0};
  }

  HitData hit;
  if (world.Hit(ray, {0.001, Infinity}, hit)) {
    Ray scattered{};
    Colour attenuation{};
    if (hit.Material->Scatter(ray, hit, attenuation, scattered)) {
      return RayColour(scattered, depth - 1, World) * attenuation;
    }
    return {0, 0, 0};
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

      // NOTE: are we supposed to do gamma-correction here? (it does look more like the book with it)
      // const auto r = (pixels[pixel_start].x);
      // const auto g = (pixels[pixel_start].y);
      // const auto b = (pixels[pixel_start].z);

      const auto r = linear_to_gamma(pixels[pixel_start].x);
      const auto g = linear_to_gamma(pixels[pixel_start].y);
      const auto b = linear_to_gamma(pixels[pixel_start].z);

      rlPixels[rl_pixel_start] = static_cast<std::uint8_t>(intensity.Clamp(r) * byteMax);
      rlPixels[rl_pixel_start + 1] = static_cast<std::uint8_t>(intensity.Clamp(g) * byteMax);
      rlPixels[rl_pixel_start + 2] = static_cast<std::uint8_t>(intensity.Clamp(b) * byteMax);
      rlPixels[rl_pixel_start + 3] = 255;
    }
  }
  return rlPixels;
}
