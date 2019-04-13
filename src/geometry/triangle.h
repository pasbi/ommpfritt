#pragma once

#include "geometry/vec2.h"
#include <array>
#include <random>

namespace omm
{

class Triangle
{
public:
  Triangle(const std::array<Vec2f, 3>& points);
  double area() const;
  template<typename Rng> Vec2f random_point(Rng&& rng) const
  {
    std::uniform_real_distribution<> urd(0.0, 1.0);
    double a = urd(rng);
    double b = urd(rng);
    if (a+b > 1.0) {
      a = 1.0 - a;
      b = 1.0 - b;
    }
    const double c = 1.0 - a - b;

    return a * points[0] + b * points[1] + c * points[2];
  }

  const std::array<Vec2f, 3> points;

private:
};

}  // namespace omm
