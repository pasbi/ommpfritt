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


Cubics::Cubics(const std::vector<Point>& points, const bool is_closed)
{
  if (is_closed && points.size() > 2) {
    m_cubics.reserve(points.size());
  } else {
    m_cubics.reserve(points.size() - 1);
  }
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

std::vector<double> find_cubic_roots(const std::array<double, 4>& coefficients) noexcept
{
  const auto& p = coefficients;
  return find_cubic_roots(std::array<double, 3>{p[1]/p[0], p[2]/p[0], p[3]/p[0]});
}

std::vector<double> find_cubic_roots(const std::array<double, 3>& coefficients) noexcept
{
  const double a = coefficients[0];
  const double b = coefficients[1];
  const double c = coefficients[2];

  const double q = (3.0*b - std::pow(a, 2.0)) / 9.0;
  const double r = (9.0*a*b - 27.0*c - 2.0*std::pow(a, 3.0)) / 54.0;
  const double d = std::pow(q, 3.0) + std::pow(r, 2.0);
  std::list<double> roots;
  if (d >= 0.0) {
    const double s = std::copysign(std::pow(std::abs(r + std::sqrt(d)), 1.0/3.0), r + std::sqrt(d));
    const double t = std::copysign(std::pow(std::abs(r - std::sqrt(d)), 1.0/3.0), r - std::sqrt(d));
    roots.push_back( -a/3.0 + s + t );
    const double imaginary = std::sqrt(3.0) * (s-t)/2.0;
    static constexpr auto eps = 10.0e-10;
    if (std::abs(imaginary) <= eps) {
      // actually, that's a double root. However we only register it once.
      roots.push_back(-a/3.0 - (s+t)/2.0);
    } else {
      // don't care about complex roots
    }
  } else {
    const double theta = std::acos(r  * std::pow(-q, -3.0/2.0));
    roots.push_back( 2.0 * std::sqrt(-q) * std::cos(     theta      /3.0) - a/3.0 );
    roots.push_back( 2.0 * std::sqrt(-q) * std::cos((theta + 2*M_PI)/3.0) - a/3.0 );
    roots.push_back( 2.0 * std::sqrt(-q) * std::cos((theta + 4*M_PI)/3.0) - a/3.0 );
  }
  return std::vector(roots.begin(), roots.end());
}
}  // namespace omm
