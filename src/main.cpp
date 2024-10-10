#include <cstddef>
#include <softrays.hpp>

#include "material.hpp"
#include "raylib.h"
#include "raytracer.hpp"
#include "shapes.hpp"
#include <raylib-cpp.hpp>

#include <cmath>
#if defined(PLATFORM_WEB)
#include <emscripten.h>
#include <emscripten/html5.h>
void RenderLoopCallback(void* arg);
#endif

// NOTE: the web version 7X faster than the native one when the native one has coverage enabled,
// but is slightly slower if build without

constexpr auto screenWidth = 400;
constexpr auto screenHeight = 300;

class Renderer {
  RayTracer raytracer;

  public:
  int Width = screenWidth;
  int Height = screenHeight;
  raylib::Image baseImage;
  raylib::Texture RenderTarget;

  void SetupViewport(int width, int height)
  {
    raytracer.ResizeViewport(width, height);
    Width = width;
    Height = height;
    baseImage.Unload();
    RenderTarget.Unload();
    baseImage = GenImageColor(width, height, raylib::Color::Black());
    // TODO: resize our texture as well (not really working at the moment, at some point we get a log "D3D12: Removing Device.", which is bad)
    baseImage.Format(PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    RenderTarget = LoadTextureFromImage(baseImage);
  }

  Renderer(int width, int height) : Width(width), Height(height)
  {
    InitWindow(screenWidth, screenHeight, "Hello, Raylib");
    SetTargetFPS(60);
    SetupViewport(width, height);
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
      SetupViewport(windowWidth, windowHeight);
    }

    BeginDrawing();
    ClearBackground(raylib::Color::DarkGray());

    raytracer.Render();
    RenderTarget.Update(raytracer.GetRGBAData().data());

    RenderTarget.Draw(0, 0);
    // NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom) (our raytracer takes care of that already)
    // target->Draw(Rectangle{0, 0, static_cast<float>(target->width), static_cast<float>(target->height)}, {0, 0}, WHITE);

    raylib::DrawText(TextFormat("%3.3f fps @ %3.4f seconds %1d spp", fps, time, raytracer.GetSamplesPerPixel()), 10, 10, 30, raylib::Color::Green());  // NOLINT

    EndDrawing();
  }

  void Start()
  {
    auto& world = raytracer.GetWorld();

    auto material_ground = std::make_shared<lambertian>(Colour{0.8, 0.8, 0.0});  // NOLINT
    auto material_center = std::make_shared<lambertian>(Colour{0.1, 0.2, 0.5});  // NOLINT
    auto material_left = std::make_shared<dialectric>(1.50);  // NOLINT
    auto material_bubble = std::make_shared<dialectric>(1.0 / 1.50);  // NOLINT
    auto material_right = std::make_shared<metal>(Colour{0.8, 0.6, 0.2}, 1.0);  // NOLINT

    world.Add(std::make_shared<Sphere>(Point3(0.0, -100.5, -1.0), 100.0, material_ground));  // NOLINT
    world.Add(std::make_shared<Sphere>(Point3(0.0, 0.0, -1.2), 0.5, material_center));  // NOLINT
    world.Add(std::make_shared<Sphere>(Point3(-1.0, 0.0, -1.0), 0.5, material_left));  // NOLINT
    world.Add(std::make_shared<Sphere>(Point3(-1.0, 0.0, -1.0), 0.4, material_bubble));  // NOLINT
    world.Add(std::make_shared<Sphere>(Point3(1.0, 0.0, -1.0), 0.5, material_right));  // NOLINT

    raytracer.lookfrom = Point3(-2, 2, 1);
    raytracer.lookat = Point3(0, 0, -1);
    raytracer.vup = Vec3(0, 1, 0);
    raytracer.vfov = 20;

#if defined(PLATFORM_WEB)
    double cssW = 0;
    double cssH = 0;
    emscripten_get_element_css_size("#canvas", &cssW, &cssH);
    SetWindowSize(static_cast<int>(cssW), static_cast<int>(cssH));
    std::cout << "window(" << GetScreenWidth() << "x" << GetScreenHeight() << ")" << std::endl;
    emscripten_set_main_loop_arg(&RenderLoopCallback, this, 60, 1);
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
  Renderer renderer(screenWidth, screenHeight);
  renderer.Start();
}

#if defined(PLATFORM_WEB)
void RenderLoopCallback(void* arg)
{
  static_cast<Renderer*>(arg)->UpdateDrawFrame();
}
#endif
