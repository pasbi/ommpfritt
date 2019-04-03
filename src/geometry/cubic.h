#pragma once

#include <array>
#include <cassert>
#include "geometry/point.h"
#include "geometry/vec2.h"

namespace omm
{

class Cubic
{
public:
  Cubic(const Point& start, const Point& end);
  Cubic(const std::array<Vec2f, 4>& points, const bool is_selected = false);

  Vec2f pos(const double t) const;
  Vec2f tangent(const double t) const;
  double length() const;
  Point evaluate(const double t) const;
  std::vector<Vec2f> interpolate(const std::size_t n) const;
  std::vector<double> cut(const Vec2f& a, const Vec2f& b) const;
  bool is_selected() const;

private:
  const std::array<Vec2f, 4> m_points;
  const bool m_is_selected;
};

std::vector<double> find_cubic_roots(const std::array<double, 4>& coefficients) noexcept;
std::vector<double> find_cubic_roots(const std::array<double, 3>& coefficients) noexcept;

}  // namespace
