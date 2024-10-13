#include "material.hpp"
#include "raytracer.hpp"
#include "shapes.hpp"
#include "utility.hpp"

#include <cmath>
#include <cstddef>
#include <iostream>
#include <raylib-cpp.hpp>

using softrays::Colour;
using softrays::Dielectric;
using softrays::Dimension2d;
using softrays::Lambertian;
using softrays::Metal;
using softrays::Point3;
using softrays::RayTracer;
using softrays::Sphere;
using softrays::Vec3;

#if defined(PLATFORM_WEB)
#include <emscripten.h>
#include <emscripten/html5.h>
void RenderLoopCallback(void* arg);
#endif

// NOTE: the web version 7X faster than the native one when the native one has coverage enabled,
// but is slightly slower if build without

constexpr Dimension2d screen{.Width = 800, .Height = 600};
constexpr auto maxFps = 60;

class Renderer {
  RayTracer raytracer;

  public:
  raylib::Image baseImage;
  raylib::Texture RenderTarget;

  Dimension2d ScreenDim{screen};

  void SetupViewport(const Dimension2d& dim)
  {
    raytracer.ResizeViewport(dim);
    baseImage.Unload();
    RenderTarget.Unload();
    baseImage = GenImageColor(dim.Width, dim.Height, raylib::Color::Black());
    // TODO: resize our texture as well (not really working at the moment, at some point we get a log "D3D12: Removing Device.", which is bad)
    baseImage.Format(PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    RenderTarget = LoadTextureFromImage(baseImage);
  }

  Renderer(const Dimension2d& dim) : ScreenDim(dim)
  {
    InitWindow(ScreenDim.Width, ScreenDim.Height, "Softrays");
    SetTargetFPS(maxFps);
    SetupViewport(ScreenDim);
  }

  void UpdateDrawFrame()
  {
    const auto time = static_cast<double>(GetFrameTime());
    const auto fps = 1.0 / time;

#if defined(PLATFORM_WEB)
    {
      double cssW = 0;
      double cssH = 0;
      emscripten_get_element_css_size("#canvas", &cssW, &cssH);
      SetWindowSize(static_cast<int>(cssW), static_cast<int>(cssH));
    }
#endif

    const auto windowWidth = GetScreenWidth();
    const auto windowHeight = GetScreenHeight();

    if (static_cast<std::size_t>(windowHeight) * static_cast<std::size_t>(windowWidth) != raytracer.GetPixelData().size()) {
      std::cout << "resizing viewport (" << windowWidth << "x" << windowHeight << ")" << raytracer.GetPixelData().size() << "\n";
      SetupViewport({.Width = windowWidth, .Height = windowHeight});
    }

    BeginDrawing();
    ClearBackground(raylib::Color::DarkGray());

    raytracer.Render();
    RenderTarget.Update(raytracer.GetRGBAData().data());

    RenderTarget.Draw(0, 0);
    // NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom) (our raytracer takes care of that already)
    // target->Draw(Rectangle{0, 0, static_cast<float>(target->width), static_cast<float>(target->height)}, {0, 0}, WHITE);

    // raylib::DrawText(TextFormat("%3.3f fps @ %3.4f seconds %1d spp", fps, time, raytracer.GetSamplesPerPixel()), 10, 10, 30, raylib::Color::Green());  // NOLINT
    std::cout << fps << "fps | " << time << " Seconds @ " << raytracer.GetSamplesPerPixel() << '\n';

    EndDrawing();
  }

  void Start()
  {
    auto& world = raytracer.GetWorld();

    // Create the Scene:

    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    auto ground_material = std::make_shared<Lambertian>(Colour{0.5, 0.5, 0.5});  // NOLINT
    world.Add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
      for (int b = -11; b < 11; b++) {
        auto choose_mat = RandomDouble();
        Point3 center(a + (0.9 * RandomDouble()), 0.2, b + (0.9 * RandomDouble()));
        if ((center - Point3(4, 0.2, 0)).Length() > 0.9) {
          if (choose_mat < 0.8) {
            // diffuse
            auto albedo = Colour::Random() * Colour::Random();
            world.Add(std::make_shared<Sphere>(center, 0.2, std::make_shared<Lambertian>(albedo)));
          } else if (choose_mat < 0.95) {
            // metal
            auto albedo = Colour::Random(0.5, 1);
            auto fuzz = RandomDouble(0, 0.5);
            world.Add(std::make_shared<Sphere>(center, 0.2, std::make_shared<Metal>(albedo, fuzz)));
          } else {
            // glass
            world.Add(std::make_shared<Sphere>(center, 0.2, std::make_shared<Dielectric>(1.5)));
          }
        }
      }
    }

    auto material1 = std::make_shared<Dielectric>(1.5);
    world.Add(std::make_shared<Sphere>(Point3(0, 1, 0), 1.0, material1));

    auto material2 = std::make_shared<Lambertian>(Colour(0.4, 0.2, 0.1));
    world.Add(std::make_shared<Sphere>(Point3(-4, 1, 0), 1.0, material2));

    auto material3 = std::make_shared<Metal>(Colour(0.7, 0.6, 0.5), 0.0);
    world.Add(std::make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));

    // Have use lower quality settings for web builds
#if defined(PLATFORM_WEB)
    raytracer.SetSamplesPerPixel(5);
    raytracer.MaxDepth = 10;
#else
    raytracer.SetSamplesPerPixel(10);
    raytracer.MaxDepth = 5;
#endif
    raytracer.FieldOfView = 20;
    raytracer.LookFrom = Point3(13, 2, 3);
    raytracer.LookAt = Point3(0, 0, 0);
    raytracer.CameraUp = Vec3(0, 1, 0);

    raytracer.DefocusAngle = 0.6;
    raytracer.FocusDistance = 10.0;
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

#if defined(PLATFORM_WEB)
    double cssW = 0;
    double cssH = 0;
    emscripten_get_element_css_size("#canvas", &cssW, &cssH);
    SetWindowSize(static_cast<int>(cssW), static_cast<int>(cssH));
    std::cout << "window(" << GetScreenWidth() << "x" << GetScreenHeight() << ")" << std::endl;
    emscripten_set_main_loop_arg(&RenderLoopCallback, this, maxFps, 1);
#else
    bool should_quit = false;
    while (!should_quit) {
      if (WindowShouldClose() || IsKeyPressed(KEY_ESCAPE)) {
        should_quit = true;
      }
      UpdateDrawFrame();
    }
#endif
  }

  ~Renderer()
  {
    RenderTarget.Unload();
    baseImage.Unload();
    CloseWindow();
  }

  Renderer(const Renderer&) = delete;
  Renderer(Renderer&&) = delete;
  Renderer& operator=(const Renderer&) = delete;
  Renderer& operator=(Renderer&&) = delete;
};

int main()
{
  Renderer renderer(screen);
  renderer.Start();
}

#if defined(PLATFORM_WEB)
void RenderLoopCallback(void* arg)
{
  static bool was_drawn = false;
  if (!was_drawn) {
    static_cast<Renderer*>(arg)->UpdateDrawFrame();
    was_drawn = true;
  }
}
#endif
