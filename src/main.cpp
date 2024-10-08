#include <cstddef>
#include <iostream>
#include <softrays.hpp>
#include <string>

#include "RenderTexture.hpp"
#include "TextureUnmanaged.hpp"
#include "raylib.h"
#include <raylib-cpp.hpp>

#include <cmath>
#include <thread>
#if defined(PLATFORM_WEB)
#include <emscripten.h>
#endif

constexpr auto screenWidth = 600;
constexpr auto screenHeight = 480;

void UpdateDrawFrame(raylib::TextureUnmanaged* target)
{
  static std::vector<Color> pixels(static_cast<std::size_t>(screenWidth * screenHeight), raylib::Color::Beige());

  const double time = GetTime();
  const auto fps = 1.0F / GetFrameTime();

  BeginDrawing();
  ClearBackground(raylib::Color::DarkGray());
  // TODO: update pixels from raytracer :D
  target->Update(pixels.data());

  raylib::DrawText(TextFormat("%3.1f", fps), 10, 10, 30, raylib::Color::Green());  // NOLINT
  // NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
  target->Draw(Rectangle{0, 0, static_cast<float>(target->width), static_cast<float>(-target->height)}, {0, 0}, WHITE);

  EndDrawing();
}

int main()
{
  InitWindow(screenWidth, screenHeight, "Hello, Raylib");

  // Create a RenderTexture2D to use as a canvas
  raylib::Image img = GenImageColor(screenWidth, screenHeight, raylib::Color::Black());
  ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  raylib::Texture target = LoadTextureFromImage(img);

#if defined(PLATFORM_WEB)
  double cssW = 0;
  double cssH = 0;
  emscripten_get_element_css_size("#canvas", &cssW, &cssH);
  main_window.SetSize({static_cast<float>(cssW), static_cast<float>(cssH)});
  emscripten_set_main_loop_arg(&UpdateDrawFrame, &target, 60, 1);
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

  CloseWindow();
}
