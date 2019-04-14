#pragma once

#include <vector>
#include "geometry/point.h"
#include <utility>
#include "geometry/cubic.h"
#include "geometry/vec2.h"
#include "geometry/boundingbox.h"
#include <cstddef>
#include <QPainterPath>
#include <random>
#include "logging.h"

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
  std::vector<double> cut(const Vec2f& a, const Vec2f& b) const;
  std::vector<double> lengths() const;
  std::size_t n_segments() const;
  const Cubic& segment(const std::size_t& segment_i) const;
  bool contains(const Vec2f& pos) const;

private:
  const std::vector<Cubic> m_cubics;
  const BoundingBox m_bounding_box;
  const bool m_is_closed;
  const QPainterPath m_path;
  mutable std::vector<double> m_lengths;
};

}  // namespace omm
