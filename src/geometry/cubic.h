#pragma once

#include <armadillo>
#include <array>
#include <cassert>

namespace omm
{

class Point;
class Cubic;

class PointOnCubic
{
public:
  PointOnCubic(const Cubic& cubic, const double t);
  const arma::vec2 pos;
  const arma::vec2 tangent;
  double rotation() const;
};

class Cubic
{
public:
  static constexpr std::size_t N = 4;
  Cubic(const Point& start, const Point& end);
  Cubic(const std::array<arma::vec2, N>& points);

  arma::vec2 pos(const double t) const;
  arma::vec2 tangent(const double t) const;
  double length() const;
  PointOnCubic evaluate(const double t) const;
  std::vector<arma::vec2> interpolate(const std::size_t n) const;

private:
  const std::array<arma::vec2, N> m_points;
};


class Cubics
{
public:
  Cubics(const std::vector<Point>& points, const bool is_closed);
  double length() const;
  void segment(const double t, std::size_t& segment_i, double& segment_t) const;
  PointOnCubic evaluate(const double t) const;

private:
  std::vector<Cubic> m_cubics;
  template<typename F> auto do_on_segment(const double t, const F& f) const
  {
    assert(0 <= t && t <= 1.0);
    double segment_t;
    std::size_t segment_i;
    segment(t, segment_i, segment_t);
    return f(m_cubics[segment_i], segment_t);
  }
};

}  // namespace
