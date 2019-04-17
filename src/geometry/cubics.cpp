#include "geometry/cubics.h"
#include "geometry/util.h"
#include "common.h"
#include <numeric>

namespace
{

std::vector<omm::Cubic> make_cubics(const std::vector<omm::Point>& points, const bool is_closed)
{
  std::list<omm::Cubic> cubics;
  if (points.size() > 0) {
    // the loop cannot handle the case points.size() == 0
    for (std::size_t i = 0; i < points.size() - 1; ++i) {
      cubics.push_back(omm::Cubic(points[i], points[i+1]));
    }
    if (is_closed && points.size() > 2) {
      cubics.push_back(omm::Cubic(points.back(), points.front()));
    }
  }
  return std::vector(cubics.begin(), cubics.end());
}

}  // namespace

namespace omm
{

Cubics::Cubics(const std::vector<Point>& points, const bool is_closed)
  : m_cubics(make_cubics(points, is_closed)), m_bounding_box(points), m_is_closed(is_closed)
  , m_path(to_path(points, is_closed))
{
}

Point Cubics::evaluate(const double path_t) const
{
  if (m_cubics.empty()) {
    return Point();
  } else {
    const auto [segment_i, segment_t] = path_to_segment(path_t);
    return segment(segment_i).evaluate(segment_t);
  }
}

std::vector<double> Cubics::cut(const Vec2f& a, const Vec2f& b) const
{
  std::list<double> ts;
  for (std::size_t segment_i = 0; segment_i < m_cubics.size(); ++segment_i) {
    for (const double segment_t : m_cubics[segment_i].cut(a, b)) {
      ts.push_back(segment_to_path(segment_i, segment_t));
    }
  }
  return std::vector(ts.begin(), ts.end());
}

std::pair<std::size_t, double> Cubics::path_to_segment(const double path_t) const
{
  if (m_cubics.empty()) { return std::pair(std::size_t(0), 0.0); }

  assert(path_t >= 0.0 && path_t <= 1.0);

  double segment_t_start = 0.0;
  double segment_t_end = lengths()[0];
  std::size_t segment_i = 0;
  const double dist = path_t * length();
  for (segment_i = 0; segment_i < m_cubics.size()-1 && segment_t_end < dist; ++segment_i) {
    segment_t_start = segment_t_end;
    segment_t_end += lengths()[segment_i+1];
  }

  const double segment_length = lengths()[segment_i];
  const double segment_t = (dist - segment_t_start) / segment_length;
  assert(segment_t >= 0.0 && segment_t <= 1.0);
  return std::pair(segment_i, segment_t);
}

double Cubics::segment_to_path(const std::size_t& segment_i, const double& segment_t) const
{
  double path_t = 0.0;
  for (std::size_t i = 0; i < segment_i; ++i) {
    path_t += lengths()[i];
  }
  path_t += segment_t * lengths()[segment_i];
  return path_t / length();
}

std::vector<double> Cubics::lengths() const
{
  if (m_lengths.empty()) {
    m_lengths = ::transform<double>(m_cubics, std::mem_fn(&Cubic::length));
  }

  return m_lengths;
}

double Cubics::length() const
{
  const auto lengths = this->lengths();
  return std::accumulate(m_lengths.begin(), m_lengths.end(), 0.0);
}

std::size_t Cubics::n_segments() const { return m_cubics.size(); }
const Cubic& Cubics::segment(const std::size_t& segment_i) const { return m_cubics[segment_i]; }

bool Cubics::contains(const Vec2f &pos) const
{
  return m_path.contains(to_qpoint(pos));
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
