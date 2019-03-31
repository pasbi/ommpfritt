#include "geometry/cubic.h"
#include "geometry/point.h"
#include <glog/logging.h>
#include <list>
#include "common.h"

namespace
{

template<typename T, std::size_t N> T evaluate(const std::array<T, N>& ps, const double t)
{
  assert(0 <= t && t <= 1.0);
  arma::vec2 p{0.0, 0.0};
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
             end.left_position(), end.position } )
{
}

Cubic::Cubic(const std::array<arma::vec2, 4>& points) : m_points(points) { }

arma::vec2 Cubic::pos(const double t) const
{
  return ::evaluate(bezier_4(m_points), t);
}

arma::vec2 Cubic::tangent(const double t) const
{
  return ::evaluate(bezier_4_derivative(m_points), t);
}

std::vector<arma::vec2> Cubic::interpolate(const std::size_t n) const
{
  assert(n >= 2);
  std::vector<arma::vec2> points;
  points.reserve(n);
  for (std::size_t i = 0; i < n; ++i) {
    points.push_back(pos(double(i)/double(n-1)));
  }
  return points;
}

double Cubic::length() const
{
  constexpr std::size_t n = 10;
  const auto points = interpolate(n);
  double length = 0.0;
  for (std::size_t i = 0; i < n - 1; ++i) { length += arma::norm(points[i] - points[i+1]); }
  return length;
}

Point Cubic::evaluate(const double t) const
{
  const auto tangent = this->tangent(t);
  return Point(pos(t), atan2(tangent(1), tangent(0)));
}

double intersect_lines(const arma::vec2& a1, const arma::vec2& a2, const arma::vec2& a3, const arma::vec2& a4)
{
  const double a = (a1(0) - a3(0)) * (a3(1) - a4(1));
  const double b = (a1(1) - a3(1)) * (a3(0) - a4(0));
  const double c = (a1(0) - a2(0)) * (a3(1) - a4(1));
  const double d = (a1(1) - a2(1)) * (a3(0) - a4(0));
  return (a-b) / (c-d);
}

std::vector<double> Cubic::cut(const arma::vec2& start, const arma::vec2& end) const
{
  if (arma::norm(start - end) < 10e-10) { return std::vector<double>(); }

  const arma::vec2 n{ end(1) - start(1), start(0) - end(0) };
  const double c = -arma::dot(start, n);

  const auto bb = bezier_4(m_points);

  const std::array<double, 4> ps = { arma::dot(n, bb[0]),  arma::dot(n, bb[1]),
                                     arma::dot(n, bb[2]),  arma::dot(n, bb[3]) + c };
  const auto roots = find_cubic_roots(ps);

  std::list<double> ts;
  std::stringstream tt;
  for (const double t : roots) {
    assert(!std::isnan(t));
    if (t >= 0.0 && t <= 1.0) {
      const arma::vec2 d = end - start;
      const arma::vec2 e = pos(t) - start;
      const double s = arma::dot(d, e) / arma::dot(d, d);
      if (s >= 0.0 && s <= 1.0) {
        ts.push_back(t);
      }
    }
  }

  auto ts_v = std::vector(ts.begin(), ts.end());
  std::sort(ts_v.begin(), ts_v.end());
  return ts_v;
}

}  // namespace omm
