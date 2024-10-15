#pragma once

#include "math.hpp"
#include "random.hpp"
#include <array>

namespace softrays {

class Perlin {
  public:
  using VecArray3 = std::array<std::array<std::array<Vec3, 2>, 2>, 2>;

  Perlin()
  {
    for (auto& rand : randvec) {
      rand = Vec3::Random(-1, 1).UnitVector();
    }

    perlin_generate_perm(perm_x);
    perlin_generate_perm(perm_y);
    perlin_generate_perm(perm_z);
  }

  [[nodiscard]] double noise(const Point3& perm) const
  {
    auto u = perm.x - std::floor(perm.x);
    auto v = perm.y - std::floor(perm.y);
    auto w = perm.z - std::floor(perm.z);

    auto i = int(std::floor(perm.x));
    auto j = int(std::floor(perm.y));
    auto k = int(std::floor(perm.z));
    VecArray3 c;

    for (int di = 0; di < 2; di++)
      for (int dj = 0; dj < 2; dj++)
        for (int dk = 0; dk < 2; dk++)
          c[di][dj][dk] = randvec[perm_x[(i + di) & 255] ^ perm_y[(j + dj) & 255] ^ perm_z[(k + dk) & 255]];

    return perlin_interp(c, u, v, w);
  }

  [[nodiscard]] double turb(const Point3& perm, int depth) const
  {
    auto accum = 0.0;
    auto temp_p = perm;
    auto weight = 1.0;

    for (int i = 0; i < depth; i++) {
      accum += weight * noise(temp_p);
      weight *= 0.5;
      temp_p *= 2;
    }

    return std::fabs(accum);
  }

  private:
  static constexpr int point_count = 256;
  std::array<Vec3, point_count> randvec;
  std::array<int, point_count> perm_x{};
  std::array<int, point_count> perm_y{};
  std::array<int, point_count> perm_z{};

  constexpr static void perlin_generate_perm(std::array<int, point_count>& perm)
  {
    for (int i = 0; i < point_count; i++)
      perm[i] = i;  // NOLINT

    permute(perm, point_count);
  }

  constexpr static void permute(std::array<int, point_count>& perm, int n)
  {
    for (int i = n - 1; i > 0; i--) {
      int target = RandomInt(0, i);
      int tmp = perm[i];
      perm[i] = perm[target];
      perm[target] = tmp;
    }
  }

  constexpr static double perlin_interp(const std::array<std::array<std::array<Vec3, 2>, 2>, 2>& c, double u, double v, double w)
  {
    auto uu = u * u * (3 - 2 * u);
    auto vv = v * v * (3 - 2 * v);
    auto ww = w * w * (3 - 2 * w);
    auto accum = 0.0;

    for (int i = 0; i < 2; i++)
      for (int j = 0; j < 2; j++)
        for (int k = 0; k < 2; k++) {
          Vec3 weight_v(u - i, v - j, w - k);
          accum += (i * uu + (1 - i) * (1 - uu))
              * (j * vv + (1 - j) * (1 - vv))
              * (k * ww + (1 - k) * (1 - ww))
              * c[i][j][k].Dot(weight_v);
        }

    return accum;
  }
};
}
