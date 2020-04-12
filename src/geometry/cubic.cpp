#include "geometry/cubic.h"
#include "geometry/point.h"
#include <list>
#include "common.h"

namespace
{

template<typename T, std::size_t N> T evaluate(const std::array<T, N>& ps, const double t)
{
  assert(0 <= t && t <= 1.0);
  omm::Vec2f p(0.0, 0.0);
  for (std::size_t j = 0; j < N; ++j) {
    p += ps[std::size_t(N-j-1)] * pow(t, j);
  }
  return p;
}

template<typename T>
std::array<T, 4> constexpr bezier_4(const std::array<T, 4>& ps) noexcept
{
  return {
    -1.0*ps[0] + 3.0*ps[1] - 3.0*ps[2] + ps[3],
     3.0*ps[0] - 6.0*ps[1] + 3.0*ps[2],
    -3.0*ps[0] + 3.0*ps[1],
     1.0*ps[0]
  };
}

template<typename T>
std::array<T, 3> constexpr bezier_4_derivative(const std::array<T, 4>& ps) noexcept
{
  return {
    -3.0*ps[0] +  9.0*ps[1] - 9.0*ps[2] + 3.0 * ps[3],
     6.0*ps[0] - 12.0*ps[1] + 6.0*ps[2],
    -3.0*ps[0] +  3.0*ps[1],
  };
}

}  // namespace

namespace omm
{

Cubic::Cubic(const Point& start, const Point& end)
  : Cubic( { start.position, start.right_position(),
             end.left_position(), end.position },
             start.is_selected && end.is_selected )
{
  assert(!start.has_nan() && !end.has_nan());
}

Cubic::Cubic(const std::array<Vec2f, 4>& points, const bool is_selected)
  : m_points(points)
  , m_is_selected(is_selected)
{ }

Vec2f Cubic::pos(const double t) const { return ::evaluate(bezier_4(m_points), t); }

Vec2f Cubic::tangent(const double t) const
{
  return -1.0/6.0 * ::evaluate(bezier_4_derivative(m_points), t);
}

std::vector<Vec2f> Cubic::interpolate(const std::size_t n) const
{
  assert(n >= 2);
  std::vector<Vec2f> points;
  points.reserve(n);
  for (std::size_t i = 0; i < n; ++i) {
    const auto pos = this->pos(static_cast<double>(i)/static_cast<double>(n-1));
    assert(!pos.has_nan());
    points.push_back(pos);
  }
  return points;
}

double Cubic::length() const
{
  constexpr std::size_t n = 10;
  const auto points = interpolate(n);
  double length = 0.0;
  for (std::size_t i = 0; i < n - 1; ++i) {
    length += (points[i] - points[i+1]).euclidean_norm();
  }
  assert(!std::isnan(length));
  return length;
}

Point Cubic::evaluate(const double t) const
{
  static constexpr auto eps = 0.001;
  Vec2f tangent = this->tangent(t);
  if (tangent.euclidean_norm() < eps) {
    if (t < eps) {
      tangent = m_points[0] - m_points[2];
    } else if (1 - t < eps) {
      tangent = m_points[1] - m_points[3];
    }
    if (tangent.euclidean_norm() < eps) {
      tangent = m_points[0] - m_points[3];
    }
    tangent /= tangent.euclidean_norm();
  }
  return Point(pos(t), PolarCoordinates(tangent), PolarCoordinates(-tangent));
}

std::vector<double> Cubic::cut(const Vec2f& start, const Vec2f& end) const
{
  if ((start - end).euclidean_norm() < 10e-10) {
    return std::vector<double>();
  }

  const Vec2f n(end.y - start.y, start.x - end.x);
  const double c = -Vec2f::dot(start, n);

  const auto bb = bezier_4(m_points);

  const std::array<double, 4> ps = { Vec2f::dot(n, bb[0]), Vec2f::dot(n, bb[1]),
                                     Vec2f::dot(n, bb[2]), Vec2f::dot(n, bb[3]) + c };
  const auto roots = find_cubic_roots(ps);

  std::list<double> ts;
  for (const double t : roots) {
    assert(!std::isnan(t));
    if (t >= 0.0 && t <= 1.0) {
      const Vec2f d = end - start;
      const Vec2f e = pos(t) - start;
      const double s = Vec2f::dot(d, e) / Vec2f::dot(d, d);
      if (s >= 0.0 && s <= 1.0) {
        ts.push_back(t);
      }
    }
  }

  auto ts_v = std::vector(ts.begin(), ts.end());
  std::sort(ts_v.begin(), ts_v.end());
  return ts_v;
}

bool Cubic::is_selected() const { return m_is_selected; }

}  // namespace omm
