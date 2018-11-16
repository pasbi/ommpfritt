#include <assert.h>

#include "geometry/boundingbox.h"

namespace
{

template<template<typename...> class Comparator>
arma::vec2 get_extremum(const std::vector<arma::vec2>& points)
{
  assert(points.size() > 0);
  auto extremum = points.front();
  for (auto i = std::next(points.cbegin()); i != points.cend(); ++i) {
    for (size_t j = 0; j < extremum.size(); ++j) {
      const auto& value = (*i)[j];
      if (Comparator<double>{}(value, extremum[j])) {
        extremum[j] = value;
      }
    }
  }
  return extremum;
}

}  // namespace

namespace omm
{

BoundingBox::BoundingBox(const std::vector<arma::vec2>& points)
  : m_top_left(get_extremum<std::less>(points)), m_bottom_right(get_extremum<std::greater>(points))
{
}

BoundingBox BoundingBox::merge(const BoundingBox& other) const
{
  return BoundingBox({
    arma::vec2 { std::min(m_top_left[0], other.m_top_left[0]),
                 std::min(m_top_left[1], other.m_top_left[1]) },
    arma::vec2 { std::max(m_bottom_right[0], other.m_bottom_right[0]),
                 std::max(m_bottom_right[1], other.m_bottom_right[1]) }
  });
}

BoundingBox BoundingBox::intersect(const BoundingBox& other) const
{
  const auto clamp = [](const auto& v, const auto& lo, const auto& hi) {
    return std::min(hi, std::max(lo, v));
  };

  return BoundingBox({
    arma::vec2 { clamp(m_top_left[0], other.m_top_left[0], other.m_bottom_right[0]),
                 clamp(m_top_left[1], other.m_top_left[1], other.m_bottom_right[1]) },
    arma::vec2 { clamp(m_bottom_right[0], other.m_top_left[0], other.m_bottom_right[0]),
                 clamp(m_bottom_right[1], other.m_top_left[1], other.m_bottom_right[1]) }
  });
}

bool BoundingBox::contains(const BoundingBox& other) const
{
  return other.contains(top_left())
      || other.contains(top_right())
      || other.contains(bottom_left())
      || other.contains(top_right());
}

bool BoundingBox::contains(const arma::vec2& point) const
{
  return m_top_left[0] <= point[0] && m_bottom_right[0] <= point[0]
      && m_top_left[1] <= point[1] && m_bottom_right[1] <= point[1];
}

arma::vec2 BoundingBox::top_left() const
{
  return m_top_left;
}

arma::vec2 BoundingBox::top_right() const
{
  return { m_bottom_right[0], m_top_left[1] };
}

arma::vec2 BoundingBox::bottom_left() const
{
  return { m_top_left[0], m_bottom_right[1] };
}

arma::vec2 BoundingBox::bottom_right() const
{
  return m_bottom_right;
}

bool BoundingBox::is_empty() const
{
  return arma::all(m_top_left == m_bottom_right);
}

double BoundingBox::left() const
{
  return top_left()[0];
}

double BoundingBox::top() const
{
  return top_left()[1];
}

double BoundingBox::right() const
{
  return bottom_right()[0];
}

double BoundingBox::bottom() const
{
  return bottom_right()[1];
}

double BoundingBox::width() const
{
  return right() - left();
}

double BoundingBox::height() const
{
  return bottom() - top();
}


}  // namespace omm
