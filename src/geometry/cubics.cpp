#include "geometry/cubics.h"
#include "common.h"

namespace omm
{

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

std::vector<double> Cubics::cut(const arma::vec2& a, const arma::vec2& b) const
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
  const double dist = path_t * length();
  double t = 0.0;
  std::size_t segment_i = 0;
  for (segment_i = 1; segment_i < m_cubics.size() && t < dist; ++segment_i) {
    t += lengths()[segment_i-1];
  }

  const double segment_t = (t - dist) / lengths()[segment_i];
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
  if (m_lengths.size() == 0) {
    m_lengths = ::transform<double>(m_cubics, std::mem_fn(&Cubic::length));
  }

  return m_lengths;
}

double Cubics::length() const
{
  const auto lengths = this->lengths();
  return std::accumulate(m_lengths.begin(), m_lengths.end(), 0.0);
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
