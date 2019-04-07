#include <cassert>

#include "geometry/boundingbox.h"
#include "common.h"
#include <algorithm>

namespace
{

double min(const std::vector<double>& ds) { return *std::min_element(ds.begin(), ds.end()); }
double max(const std::vector<double>& ds) { return *std::max_element(ds.begin(), ds.end()); }

}

namespace omm
{

BoundingBox::BoundingBox(const std::vector<Vec2f>& points)
  : BoundingBox( ::transform<double>(points, [](const Vec2f& v) { return v.x; }),
                 ::transform<double>(points, [](const Vec2f& v) { return v.y; }) ) {}

BoundingBox::BoundingBox(const std::vector<double>& xs, const std::vector<double>& ys)
  : Rectangle(Vec2f(min(xs), min(ys)), Vec2f(max(xs), max(ys))) {}

BoundingBox BoundingBox::merge(const BoundingBox& other) const
{
  return BoundingBox({
    Vec2f( std::min(top_left().x, other.top_left().x),
           std::min(top_left().y, other.top_left().y) ),
    Vec2f( std::max(bottom_right().x, other.bottom_right().x),
           std::max(bottom_right().y, other.bottom_right().y) )
  });
}

BoundingBox BoundingBox::intersect(const BoundingBox& other) const
{
  return BoundingBox({
    Vec2f( std::clamp(top_left().x, other.top_left().x, other.bottom_right().x),
           std::clamp(top_left().y, other.top_left().y, other.bottom_right().y) ),
    Vec2f( std::clamp(bottom_right().x, other.top_left().x, other.bottom_right().x),
           std::clamp(bottom_right().y, other.top_left().y, other.bottom_right().y) )
  });
}

bool BoundingBox::contains(const BoundingBox& other) const
{
  return other.contains(top_left())
      || other.contains(top_right())
      || other.contains(bottom_left())
      || other.contains(top_right());
}

std::ostream& operator<<(std::ostream &ostream, const BoundingBox &bb)
{
  ostream << "BoundingBox[" << bb.top_left() << ", " << bb.width() << "x" << bb.height() << "]";
  return ostream;
}

}  // namespace omm
