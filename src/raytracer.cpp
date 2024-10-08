#include <raytracer.hpp>

void RayTracer::Render()
{
  auto focal_length = 1.0;
  auto viewport_height = 2.0;  // NOLINT
  auto viewport_width = viewport_height * (static_cast<double>(width) / height);
  auto camera_center = Point3(0, 0, 0);

  // Calculate the vectors across the horizontal and down the vertical viewport edges.
  auto viewport_u = Vec3(viewport_width, 0, 0);
  auto viewport_v = Vec3(0, -viewport_height, 0);

  // Calculate the horizontal and vertical delta vectors from pixel to pixel.
  auto pixel_delta_u = viewport_u / width;
  auto pixel_delta_v = viewport_v / height;

  // Calculate the location of the upper left pixel.
  auto viewport_upper_left = camera_center
      - Vec3(0, 0, focal_length) - viewport_u / 2.0 - viewport_v / 2.0;
  auto pixel00_loc = viewport_upper_left + ((pixel_delta_u + pixel_delta_v) * 0.5);

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      auto pixel_center = pixel00_loc + (pixel_delta_u * x) + (pixel_delta_v * y);
      auto ray_direction = pixel_center - camera_center;
      Ray r(camera_center, ray_direction);

      Colour pixel_colour = RayColour(r);
      pixels[(y * width) + x] = pixel_colour;
    }
  }
}

double hit_sphere(const Point3& center, double radius, const RayTracer::Ray& ray)
{
  const Vec3 o_c = center - ray.Origin;
  const auto a = ray.Direction.dot(ray.Direction);
  const auto b = -2.0 * ray.Direction.dot(o_c);
  const auto c = o_c.dot(o_c) - (radius * radius);
  const auto discriminant = (b * b) - (4 * a * c);
  return discriminant < 0 ? -1.0 : ((-b - std::sqrt(discriminant)) / (a * 2.0));
}

Colour RayTracer::RayColour(const Ray& ray) const
{
  auto t = hit_sphere(Point3(0, 0, -1), 0.5, ray);
  if (t > 0.0) {
    Vec3 N = (ray.at(t) - Vec3(0, 0, -1)).unit_vector();
    return Colour{N.x + 1, N.y + 1, N.z + 1} * 0.5;
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
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      const auto pixel_start = static_cast<std::size_t>((y * width) + x);
      const auto rl_pixel_start = static_cast<std::size_t>(((y * width) + x) * 4);
      rlPixels[rl_pixel_start] = static_cast<std::uint8_t>(pixels[pixel_start].x * ColourByteFactor);
      rlPixels[rl_pixel_start + 1] = static_cast<std::uint8_t>(pixels[pixel_start].y * ColourByteFactor);
      rlPixels[rl_pixel_start + 2] = static_cast<std::uint8_t>(pixels[pixel_start].z * ColourByteFactor);
      rlPixels[rl_pixel_start + 3] = 255;
    }
  }
  return rlPixels;
}
