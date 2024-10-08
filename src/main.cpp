#include <cstddef>
#include <cstdint>
#include <softrays.hpp>

#include "TextureUnmanaged.hpp"
#include "raylib.h"
#include "raytracer.hpp"
#include <raylib-cpp.hpp>

#include <cmath>
#include <thread>
#if defined(PLATFORM_WEB)
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

constexpr auto screenWidth = 1200;
constexpr auto screenHeight = 675;

void UpdateDrawFrame(raylib::TextureUnmanaged* target)
{
  // static std::vector<std::uint8_t> pixels(static_cast<std::size_t>(screenWidth * screenHeight * 4), 90);
  static RayTracer raytracer;

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
    raytracer.ResizeViewport(windowWidth, windowHeight);

    // TODO: resize our texture as well (not really working at the moment, at some point we get a log "D3D12: Removing Device.", which is bad)
    target->Unload();
    // std::this_thread::sleep_for(std::chrono::duration<double>(1));
    raylib::Image img = GenImageColor(windowWidth, windowHeight, raylib::Color::Black());
    img.Format(PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    // std::cout << "generating image data\n";
    // std::this_thread::sleep_for(std::chrono::duration<double>(1));
    target->Load(img);
    // std::cout << "texture size (" << target->width << "x" << target->height << ")\n";
    // std::this_thread::sleep_for(std::chrono::duration<double>(1));
  }

  BeginDrawing();
  ClearBackground(raylib::Color::DarkGray());

  raytracer.Render();
  target->Update(raytracer.GetRGBAData().data());

  target->Draw(0, 0);
  // NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
  // target->Draw(Rectangle{0, 0, static_cast<float>(target->width), static_cast<float>(target->height)}, {0, 0}, WHITE);

  raylib::DrawText(TextFormat("%3.1f", fps), 10, 10, 30, raylib::Color::Green());  // NOLINT

  EndDrawing();
}

#if defined(PLATFORM_WEB)
void UpdateEmscriptenFrame(void* target)
{
  UpdateDrawFrame(static_cast<raylib::TextureUnmanaged*>(target));
}
#endif

int main()
{
  InitWindow(screenWidth, screenHeight, "Hello, Raylib");
  {
    // Create a RenderTexture2D to use as a canvas
    raylib::Image img = GenImageColor(screenWidth, screenHeight, raylib::Color::Black());
    img.Format(PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    raylib::Texture target = LoadTextureFromImage(img);

#if defined(PLATFORM_WEB)
    double cssW = 0;
    double cssH = 0;
    emscripten_get_element_css_size("#canvas", &cssW, &cssH);
    SetWindowSize(static_cast<int>(cssW), static_cast<int>(cssH));
    emscripten_set_main_loop_arg(&UpdateEmscriptenFrame, &target, 60, 1);
#else
    SetTargetFPS(60);

    bool should_quit = false;
    while (!should_quit) {
      if (WindowShouldClose() || IsKeyPressed(KEY_ESCAPE)) {
        should_quit = true;
      }
      UpdateDrawFrame(&target);
    }
#endif
  }
  CloseWindow();
}
