#include "bvh.hpp"
#include "material.hpp"
#include "math.hpp"
#include "raytracer.hpp"
#include "shapes.hpp"
#include "texture.hpp"
#include "transforms.hpp"
#include "utility.hpp"
#include "volumes.hpp"

#include <Image.hpp>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <memory>
#include <raylib-cpp.hpp>
#include <raylib.h>

using namespace softrays;

#if defined(PLATFORM_WEB)
#include <emscripten.h>
#include <emscripten/html5.h>
void RenderLoopCallback(void* arg);
#endif

// NOTE: the web version 7X faster than the native one when the native one has coverage enabled,
// but is slightly slower if build without

constexpr Dimension2d screen{.Width = 800, .Height = 600};
constexpr Dimension2d renderDim{.Width = 800, .Height = 600};
constexpr auto maxFps = 60;

void test_scene(HittableList& world)
{
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
  // auto checker = std::make_shared<softrays::CheckeredTexture>(0.32, Colour(.2, .3, .1), Colour(.9, .9, .9));
  auto uvtest_texture = std::make_shared<softrays::ImageTexture>("resources/uvtest.png");
  auto ground_material = std::make_shared<Lambertian>(uvtest_texture);  // NOLINT
  // auto ground_material = std::make_shared<Lambertian>(std::make_shared<softrays::ImageTexture>("resources/earthmap.png"));  // NOLINT
  // auto ground_material = std::make_shared<Lambertian>(earth);  // NOLINT
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

  auto material2 = std::make_shared<Lambertian>(std::make_shared<NoiseTexture>(Colour(0.4, 0.2, 0.1), 4));
  world.Add(std::make_shared<Sphere>(Point3(-4, 1, 0), 1.0, ground_material));

  auto material3 = std::make_shared<Metal>(Colour(0.7, 0.6, 0.5), 0.0);
  world.Add(std::make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));

  world.Add(std::make_shared<Quad>(Point3(0, 2, -2), Vec3{2, 0, 0}, Vec3{0, 2, 0}, ground_material));

  world.Add(std::make_shared<Triangle>(Point3(2, 2, 0), Vec3{4, 2, 0}, Vec3{2, 4, 0}, ground_material));

  world.Add(std::make_shared<Annulus>(Point3(-3, 2, -2), Vec3{2, 0, 0}, Vec3{0, 2, 0}, 0.5, ground_material));
  // world.Add(std::make_shared<Ellipse>(Point3(-3, 2, -2), Vec3{2, 0, 0}, Vec3{0, 2, 0}, ground_material));

  auto textured_light_mat = std::make_shared<DiffuseLight>(3, uvtest_texture);
  std::shared_ptr<Hittable> transformed_light_box = MakeBoxQuadList(Point3{-5, -4, 4}, Point3{-2, 1, -1}, textured_light_mat);
  transformed_light_box = std::make_shared<Rotate_Y>(transformed_light_box, -18);
  transformed_light_box = std::make_shared<Translate>(transformed_light_box, Vec3{0, 0, -1});
  world.Add(transformed_light_box);
  world.Add(std::make_shared<ConstantMedium>(std::make_shared<Sphere>(Point3(-6, 4, 1), 1.75, material2), 0.3, uvtest_texture));

  auto light = std::make_shared<DiffuseLight>(Colour(7, 3, 3));
  world.Add(make_shared<Quad>(Point3(0, 10, 0), Vec3(10, 0, 0), Vec3(0, 0, 10), light));

  // TODO: I hate the way this currently works, we shouldn't be overwriting the list, it should be a bvh from the start...
  world = HittableList(std::make_shared<BVH>(world));

  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
}

// Final scene from book 2
void final_scene(RayTracer& cam)
{
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
  HittableList boxes1;
  auto ground = std::make_shared<Lambertian>(Colour(0.48, 0.83, 0.53));

  int boxes_per_side = 20;
  for (int i = 0; i < boxes_per_side; i++) {
    for (int j = 0; j < boxes_per_side; j++) {
      auto w = 100.0;
      auto x0 = -1000.0 + i * w;
      auto z0 = -1000.0 + j * w;
      auto y0 = 0.0;
      auto x1 = x0 + w;
      auto y1 = RandomDouble(1, 101);
      auto z1 = z0 + w;

      boxes1.Add(MakeBoxQuadList(Point3(x0, y0, z0), Point3(x1, y1, z1), ground));
    }
  }

  HittableList world;

  world.Add(std::make_shared<BVH>(boxes1));

  auto light = std::make_shared<DiffuseLight>(Colour(7, 7, 7));
  world.Add(make_shared<Quad>(Point3(123, 554, 147), Vec3(300, 0, 0), Vec3(0, 0, 265), light));

  auto center1 = Point3(400, 400, 200);
  // auto center2 = center1 + Vec3(30, 0, 0);
  auto sphere_material = std::make_shared<Lambertian>(Colour(0.7, 0.3, 0.1));
  world.Add(std::make_shared<Sphere>(center1, 50, sphere_material));

  world.Add(std::make_shared<Sphere>(Point3(260, 150, 45), 50, std::make_shared<Dielectric>(1.5)));
  world.Add(std::make_shared<Sphere>(
      Point3(0, 150, 145), 50, std::make_shared<Metal>(Colour(0.8, 0.8, 0.9), 1.0)));

  auto boundary = std::make_shared<Sphere>(Point3(360, 150, 145), 70, std::make_shared<Dielectric>(1.5));
  world.Add(boundary);
  world.Add(std::make_shared<ConstantMedium>(boundary, 0.2, Colour(0.2, 0.4, 0.9)));
  boundary = std::make_shared<Sphere>(Point3(0, 0, 0), 5000, std::make_shared<Dielectric>(1.5));
  world.Add(std::make_shared<ConstantMedium>(boundary, .0001, Colour(1, 1, 1)));

  auto emat = std::make_shared<Lambertian>(std::make_shared<ImageTexture>("resources/uvtest.png"));
  world.Add(std::make_shared<Sphere>(Point3(400, 200, 400), 100, emat));
  auto pertext = std::make_shared<NoiseTexture>(0.2);
  world.Add(std::make_shared<Sphere>(Point3(220, 280, 300), 80, std::make_shared<Lambertian>(pertext)));

  HittableList boxes2;
  auto white = std::make_shared<Lambertian>(Colour(.73, .73, .73));
  int ns = 1000;
  for (int j = 0; j < ns; j++) {
    boxes2.Add(make_shared<Sphere>(Point3::Random(0, 165), 10, white));
  }

  world.Add(make_shared<Translate>(
      std::make_shared<Rotate_Y>(
          std::make_shared<BVH>(boxes2), 15),
      Vec3(-100, 270, 395)));

  cam.BackgroundColour = Colour(0, 0, 0);

  cam.FieldOfView = 40;
  cam.LookFrom = Point3(478, 278, -600);
  cam.LookAt = Point3(278, 278, 0);
  cam.CameraUp = Vec3(0, 1, 0);

  cam.DefocusAngle = 0;
  cam.GetWorld() = world;
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
}

class Renderer {
  RayTracer raytracer;

  public:
  raylib::Image baseImage;
  raylib::Texture RenderTarget;

  Dimension2d ScreenDim{screen};
  Dimension2d RenderDim{renderDim};
  bool RenderAtScreenDim = false;
  bool IncrementalRender = true;
  std::size_t LastRenderedPixel{};
  double LastCompleteDrawTime{};

  void SetupViewport(const Dimension2d& dim)
  {
    RenderDim = dim;
    raytracer.ResizeViewport(RenderDim);
    baseImage.Unload();
    RenderTarget.Unload();
    baseImage = GenImageColor(RenderDim.Width, RenderDim.Height, raylib::Color::Black());
    // TODO: resize our texture as well (not really working at the moment, at some point we get a log "D3D12: Removing Device.", which is bad)
    baseImage.Format(PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    RenderTarget = LoadTextureFromImage(baseImage);
  }

  Renderer(const Dimension2d& dim) : ScreenDim(dim)
  {
    InitWindow(ScreenDim.Width, ScreenDim.Height, "Softrays");
    SetTargetFPS(maxFps);
    SetupViewport(renderDim);
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

    if (static_cast<std::size_t>(windowHeight) * static_cast<std::size_t>(windowWidth) != (static_cast<std::size_t>(ScreenDim.Width * ScreenDim.Height))) {
      std::cout << "resizing viewport (" << windowWidth << "x" << windowHeight << ")(" << ScreenDim.Width << "x" << ScreenDim.Height << ")\n";
      ScreenDim.Width = windowWidth;
      ScreenDim.Height = windowHeight;
      if (RenderAtScreenDim) {
        SetupViewport({.Width = windowWidth, .Height = windowHeight});
      }
    }

    BeginDrawing();
    ClearBackground(raylib::Color::DarkGray());

    if (IncrementalRender) {
      if (LastRenderedPixel >= (static_cast<std::size_t>(RenderDim.Width) * static_cast<std::size_t>(RenderDim.Height))) {
        LastRenderedPixel = 0;
        std::cout << "Frame Render took:" << GetTime() - LastCompleteDrawTime << "s\n";
        LastCompleteDrawTime = GetTime();
      }
      int nextY = static_cast<int>(LastRenderedPixel / static_cast<std::size_t>(RenderDim.Width));
      int nextX = static_cast<int>(LastRenderedPixel % static_cast<std::size_t>(RenderDim.Width));
      LastRenderedPixel += static_cast<std::size_t>(RenderDim.Width) / 4;
      raytracer.Render(nextX, nextY, nextX + (renderDim.Width / 4), nextY + 1);
    } else {
      raytracer.Render();
    }

    RenderTarget.Update(raytracer.GetRGBAData().data());

    RenderTarget.Draw(Rectangle{0, 0, static_cast<float>(RenderDim.Width), static_cast<float>(RenderDim.Height)}, Rectangle{0, 0, static_cast<float>(ScreenDim.Width), static_cast<float>(ScreenDim.Height)});
    // NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom) (our raytracer takes care of that already)
    // target->Draw(Rectangle{0, 0, static_cast<float>(target->width), static_cast<float>(target->height)}, {0, 0}, WHITE);

    if (fps < 1.0) {
      std::cout << fps << "fps | " << time << " Seconds @ " << raytracer.GetSamplesPerPixel() << '\n';
    } else if (!std::isinf(fps) && !IncrementalRender) {
      raylib::DrawText(TextFormat("%3.3f fps @ %3.4f seconds %1d spp", fps, time, raytracer.GetSamplesPerPixel()), 10, 10, 30, raylib::Color::Green());  // NOLINT
    }

    EndDrawing();
  }

  void Start()
  {
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

    // Have use lower quality settings for web builds
#if defined(PLATFORM_WEB)
    raytracer.SetSamplesPerPixel(200);
    raytracer.MaxDepth = 30;
#else
    raytracer.SetSamplesPerPixel(500);
    raytracer.MaxDepth = 50;
#endif
    raytracer.FieldOfView = 50;
    raytracer.LookFrom = Point3(-4, 4, 13);
    raytracer.LookAt = Point3(0, 2, 0);
    raytracer.CameraUp = Vec3(0, 1, 0);

    raytracer.DefocusAngle = 0.1;
    raytracer.FocusDistance = 10.0;
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

    // Create scene:
    final_scene(raytracer);
    // test_scene(raytracer.GetWorld());
    // raytracer.BackgroundColour = {0, 0, 0};
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
  if (static_cast<Renderer*>(arg)->IncrementalRender || !was_drawn) {
    static_cast<Renderer*>(arg)->UpdateDrawFrame();
    was_drawn = true;
  }
}
#endif
