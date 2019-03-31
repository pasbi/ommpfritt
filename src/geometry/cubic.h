#pragma once

#include <armadillo>
#include <array>
#include <cassert>

#include "geometry/point.h"

namespace omm
{

class Cubic
{
public:
  Cubic(const Point& start, const Point& end);
  Cubic(const std::array<arma::vec2, 4>& points);

  arma::vec2 pos(const double t) const;
  arma::vec2 tangent(const double t) const;
  double length() const;
  Point evaluate(const double t) const;
  std::vector<arma::vec2> interpolate(const std::size_t n) const;
  std::vector<double> cut(const arma::vec2& a, const arma::vec2& b) const;

private:
  const std::array<arma::vec2, 4> m_points;
};

std::vector<double> find_cubic_roots(const std::array<double, 4>& coefficients) noexcept;
std::vector<double> find_cubic_roots(const std::array<double, 3>& coefficients) noexcept;

}  // namespace
