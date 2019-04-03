#include <cassert>

#include "geometry/boundingbox.h"
#include "common.h"

namespace omm
{

BoundingBox::BoundingBox(const std::vector<Vec2f>& points)
{
  const auto xs = ::transform<double>(points, [](const Vec2f& v) { return v.x; });
  const auto ys = ::transform<double>(points, [](const Vec2f& v) { return v.y; });
  m_top_left = Vec2f( *std::min_element(xs.begin(), xs.end()),
                      *std::min_element(ys.begin(), ys.end()) );
  m_bottom_right = Vec2f( *std::max_element(xs.begin(), xs.end()),
                          *std::max_element(ys.begin(), ys.end()) );
}

BoundingBox BoundingBox::merge(const BoundingBox& other) const
{
  return BoundingBox({
    Vec2f( std::min(m_top_left[0], other.m_top_left[0]),
           std::min(m_top_left[1], other.m_top_left[1]) ),
    Vec2f( std::max(m_bottom_right[0], other.m_bottom_right[0]),
           std::max(m_bottom_right[1], other.m_bottom_right[1]) )
  });
}

BoundingBox BoundingBox::intersect(const BoundingBox& other) const
{
  return BoundingBox({
    Vec2f( std::clamp(m_top_left[0], other.m_top_left[0], other.m_bottom_right[0]),
           std::clamp(m_top_left[1], other.m_top_left[1], other.m_bottom_right[1]) ),
    Vec2f( std::clamp(m_bottom_right[0], other.m_top_left[0], other.m_bottom_right[0]),
           std::clamp(m_bottom_right[1], other.m_top_left[1], other.m_bottom_right[1]) )
  });
}

bool BoundingBox::contains(const BoundingBox& other) const
{
  return other.contains(top_left())
      || other.contains(top_right())
      || other.contains(bottom_left())
      || other.contains(top_right());
}

bool BoundingBox::contains(const Vec2f& point) const
{
  return m_top_left[0] <= point[0] && m_bottom_right[0] <= point[0]
      && m_top_left[1] <= point[1] && m_bottom_right[1] <= point[1];
}

Vec2f BoundingBox::top_left() const
{
  return m_top_left;
}

Vec2f BoundingBox::top_right() const
{
  return Vec2f(m_bottom_right[0], m_top_left[1]);
}

Vec2f BoundingBox::bottom_left() const
{
  return Vec2f(m_top_left[0], m_bottom_right[1]);
}

Vec2f BoundingBox::bottom_right() const
{
  return m_bottom_right;
}

bool BoundingBox::is_empty() const
{
  return m_top_left == m_bottom_right;
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
