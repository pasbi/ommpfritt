#pragma once

#include "geometry/vec2.h"
#include <array>
#include <random>
#include "geometry/edge.h"

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

  bool contains(const Vec2f& p) const;
  bool has_vertex(const Vec2f& p, std::size_t *index = nullptr) const;
  bool has_edge(const Edge& edge) const;
  bool crosses_edge(const Edge& edge, Edge &crossing) const;
  bool is_neighbor(const Triangle& other) const;
  std::vector<Edge> edges() const;

  static void swap_diagonal(Triangle &a, Triangle &b);

  std::array<Vec2f, 3> points;
  bool operator==(const Triangle& other) const;
  bool operator!=(const Triangle& other) const;

  bool marked = false;

private:
};

std::ostream& operator <<(std::ostream& stream, const Triangle& triangle);

}  // namespace omm
