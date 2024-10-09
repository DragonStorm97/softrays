#include <cstddef>
#include <cstdint>
#include <softrays.hpp>

#include "TextureUnmanaged.hpp"
#include "raylib.h"
#include "raytracer.hpp"
#include "shapes.hpp"
#include <raylib-cpp.hpp>

#include <cmath>
#include <thread>
#if defined(PLATFORM_WEB)
#include <emscripten.h>
#include <emscripten/html5.h>
void RenderLoopCallback(void* arg);
#endif

constexpr auto screenWidth = 800;
constexpr auto screenHeight = 600;

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
    const double time = GetTime();
    const auto fps = 1.0 / static_cast<double>(GetFrameTime());

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

    raylib::DrawText(TextFormat("%3.1f", fps), 10, 10, 30, raylib::Color::Green());  // NOLINT

    EndDrawing();
  }

  void Start()
  {
    auto& world = raytracer.GetWorld();
    world.Add(std::make_shared<Sphere>(Point3(0, 0, -1), 0.5));
    world.Add(std::make_shared<Sphere>(Point3(0, -100.5, -1), 100));

#if defined(PLATFORM_WEB)
    double cssW = 0;
    double cssH = 0;
    emscripten_get_element_css_size("#canvas", &cssW, &cssH);
    SetWindowSize(static_cast<int>(cssW), static_cast<int>(cssH));
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
