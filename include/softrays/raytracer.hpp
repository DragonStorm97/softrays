#pragma once

#include "hittable.hpp"
#include "math.hpp"
#include "utility.hpp"
#include <cstdint>

namespace softrays {
class RayTracer {
  public:
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
  Point3 CameraPosition{0, 0, 0};

  Colour BackgroundColour{0.7, 0.8, 1.0};
  int MaxDepth = 50;  // Maximum number of bounces
  double FieldOfView = 90;  // Vertical view angle (field of view)

  Point3 LookFrom = Point3(0, 0, 0);  // Point camera is looking from
  Point3 LookAt = Point3(0, 0, -1);  // Point camera is looking at
  Vec3 CameraUp = Vec3(0, 1, 0);  // Camera-relative "up" direction

  double DefocusAngle = 0;  // Variation angle of rays through each pixel
  double FocusDistance = 10;  // Distance from camera lookfrom point to plane of perfect focus

  private:
  Dimension2d ViewportDimensions{.Width = 600, .Height = 400};  // Rendered Image Dimensions
  int SamplesPerPixel = 100;  // Count of random samples for each pixel
  double PixelSamplesScale = 1.0 / SamplesPerPixel;  // Color scale factor for a sum of pixel samples
  Vec3 Camera_u, Camera_v, Camera_w;  // Camera frame basis vectors
  Vec3 DefocusDisk_u;  // Defocus disk horizontal radius
  Vec3 DefocusDisk_v;  // Defocus disk vertical radius

  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

  HittableList World;
  std::vector<std::uint8_t> rlPixels;
  std::vector<Colour> PixelData;

  [[nodiscard]] Point3 DefocusDiskSample() const noexcept;
  [[nodiscard]] Colour RayColour(const Ray& ray, int depth, const class Hittable& World) const;

  public:
  [[nodiscard]] int GetSamplesPerPixel() const noexcept
  {
    return SamplesPerPixel;
  }

  void SetSamplesPerPixel(int spp) noexcept
  {
    SamplesPerPixel = spp;
    PixelSamplesScale = 1.0 / spp;
  }

  [[nodiscard]] HittableList& GetWorld() { return World; }
  void ResizeViewport(const Dimension2d& dim);

  [[nodiscard]] Ray GetRayForPixel(int x, int y, const Vec3& pixel00_loc, const Vec3& pixel_delta_u, const Vec3& pixel_delta_v) const;
  [[nodiscard]] const std::vector<std::uint8_t>& GetRGBAData();
  void SetupCamera();
  void Render(int fromX, int fromY, int toX, int toY);
  void Render();
  [[nodiscard]] const std::vector<Colour>& GetPixelData() const { return PixelData; }
};
}
