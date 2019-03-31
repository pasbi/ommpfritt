#pragma once

#include <vector>
#include "geometry/point.h"
#include <utility>
#include "geometry/cubic.h"
#include <armadillo>
#include <cstddef>

namespace omm
{

class Cubics
{
public:
  Cubics(const std::vector<Point>& points, const bool is_closed);
  double length() const;
  double distance(double t);
  std::pair<std::size_t, double> path_to_segment(const double path_t) const;
  double segment_to_path(const std::size_t& segment_i, const double& segment_t) const;
  Point evaluate(const double path_t) const;
  std::vector<double> cut(const arma::vec2& a, const arma::vec2& b) const;
  std::vector<double> lengths() const;
  std::size_t n_segments() const;
  const Cubic& segment(const std::size_t& segment_i) const;

private:
  const std::vector<Cubic> m_cubics;
  mutable std::vector<double> m_lengths;
};

}  // namespace omm
