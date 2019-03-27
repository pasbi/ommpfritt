#include "geometry/cubic.h"
#include "geometry/point.h"
#include <glog/logging.h>

namespace
{
template<typename Fs, typename Points>
arma::vec2 accumulate(const Fs& fs, const Points& points, const double t)
{
  assert(fs.size() == points.size());
  arma::vec2 v{0.0, 0.0};
  for (std::size_t i = 0; i < fs.size(); ++i) { v += fs[i](t) * points[i]; }
  return v;
}

}  // namespace

namespace omm
{

Cubic::Cubic(const Point& start, const Point& end)
  : Cubic( { start.position, 3 * start.right_tangent.to_cartesian(),
             end.position, -3 * end.left_tangent.to_cartesian() } )
{
}

Cubic::Cubic(const std::array<arma::vec2, N>& points) : m_points(points) { }

arma::vec2 Cubic::pos(const double t) const
{
  static const std::array<std::function<double(double)>, N> base_polynomials {
    [](const double t) { return  2*t*t*t - 3*t*t + 0*t + 1; },
    [](const double t) { return  1*t*t*t - 2*t*t + 1*t + 0; },
    [](const double t) { return -2*t*t*t + 3*t*t + 0*t + 0; },
    [](const double t) { return  1*t*t*t - 1*t*t + 0*t + 0; }
  };
  assert(0 <= t && t <= 1.0);
  return accumulate(base_polynomials, m_points, t);
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

arma::vec2 Cubic::tangent(const double t) const
{
  static const std::array<std::function<double(double)>, N> base_polynomials_derivatives {
    [](const double t) { return  6*t*t - 6*t + 0; },
    [](const double t) { return  3*t*t - 4*t + 1; },
    [](const double t) { return -6*t*t + 6*t + 0; },
    [](const double t) { return  3*t*t - 2*t + 0; }
  };
  assert(0 <= t && t <= 1.0);
  return accumulate(base_polynomials_derivatives, m_points, t);
}

Point Cubic::evaluate(const double t) const
{
  const auto tangent = this->tangent(t);
  return Point(pos(t), atan2(tangent(1), tangent(0)));
}

Cubics::Cubics(const std::vector<Point>& points, const bool is_closed)
{
  m_cubics.reserve(points.size() - 1);
  for (std::size_t i = 0; i < points.size() - 1; ++i) {
    m_cubics.push_back(Cubic(points[i], points[i+1]));
  }
  if (is_closed && points.size() > 2) {
    m_cubics.push_back(Cubic(points.back(), points.front()));
  }
}

Point Cubics::evaluate(const double t) const
{
  double segment_t = -1.0;
  std::size_t segment_i = m_cubics.size();
  assert(0 <= t && t <= 1.0);
  const double t_length = t * length();
  if (t == 1.0) {
    segment_i = m_cubics.size() - 1;
    segment_t = 1.0;
  } else {
    double length_accu = 0.0;
    double current_length = 0.0;
    for (std::size_t i = 0; i < m_cubics.size(); ++i) {
      current_length = m_cubics[i].length();
      if (t_length < length_accu + current_length) {
        segment_i = i;
        break;
      } else {
        length_accu += current_length;
      }
    }
    assert(segment_i < m_cubics.size());
    segment_t = (t_length - length_accu) / current_length;
  }

  return m_cubics[segment_i].evaluate(segment_t);
}

double Cubics::length() const
{
  const auto add = [](const double accu, const auto& cubic) { return accu + cubic.length(); };
  return std::accumulate(m_cubics.begin(), m_cubics.end(), 0.0, add);
}

}  // namespace omm
