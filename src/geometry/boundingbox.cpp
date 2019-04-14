#include <cassert>

#include "geometry/boundingbox.h"
#include "common.h"
#include <algorithm>

namespace
{

double min(const std::vector<double>& ds)
{
  if (ds.empty()) {
    return 0.0;
  } else {
    return *std::min_element(ds.begin(), ds.end());
  }
}
double max(const std::vector<double>& ds)
{
  if (ds.empty()) {
    return 0.0;
  } else {
    return *std::max_element(ds.begin(), ds.end());
  }
}

std::vector<omm::Vec2f> get_all_control_points(const std::vector<omm::Point>& points)
{
  std::list<omm::Vec2f> control_points;
  for (auto&& p : points) {
    control_points.push_back(p.left_position());
    control_points.push_back(p.position);
    control_points.push_back(p.right_position());
  }
  return std::vector(control_points.begin(), control_points.end());
}

}

namespace omm
{

BoundingBox::BoundingBox(const std::vector<Vec2f>& points)
  : BoundingBox( ::transform<double>(points, [](const Vec2f& v) { return v.x; }),
                 ::transform<double>(points, [](const Vec2f& v) { return v.y; }) ) {}

BoundingBox::BoundingBox(const std::vector<double>& xs, const std::vector<double>& ys)
  : Rectangle(Vec2f(min(xs), min(ys)), Vec2f(max(xs), max(ys))) {}

BoundingBox::BoundingBox(const std::vector<Point>& points)
  : BoundingBox(get_all_control_points(points)) {}

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

BoundingBox& BoundingBox::operator |=(const BoundingBox& other)
{
  *this = *this | other;
  return *this;
}

BoundingBox& BoundingBox::operator &=(const BoundingBox& other)
{
  *this = *this & other;
  return *this;
}

BoundingBox operator|(const BoundingBox& a, const BoundingBox& b)
{
  return BoundingBox({  Vec2f( std::min(a.top_left().x, b.top_left().x),
                               std::min(a.top_left().y, b.top_left().y) ),
                        Vec2f( std::max(a.bottom_right().x, b.bottom_right().x),
                               std::max(a.bottom_right().y, b.bottom_right().y) )
                      });
}

BoundingBox operator&(const BoundingBox& a, const BoundingBox& b)
{
  return BoundingBox({ Vec2f( std::clamp(a.top_left().x, b.top_left().x, b.bottom_right().x),
                               std::clamp(a.top_left().y, b.top_left().y, b.bottom_right().y) ),
                        Vec2f( std::clamp(a.bottom_right().x, b.top_left().x, b.bottom_right().x),
                               std::clamp(a.bottom_right().y, b.top_left().y, b.bottom_right().y) )
                      });
}

}  // namespace omm
