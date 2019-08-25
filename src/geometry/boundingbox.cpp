#include <cassert>

#include "geometry/boundingbox.h"
#include "common.h"
#include <algorithm>
#include "scene/scene.h"

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
  : Rectangle(Vec2f(min(xs), min(ys)), Vec2f(max(xs), max(ys)), xs.empty())
{
  assert(xs.empty() == ys.empty());
}

BoundingBox::BoundingBox(const std::vector<Point>& points)
  : BoundingBox(get_all_control_points(points)) {}

bool BoundingBox::contains(const BoundingBox& other) const
{
  if (is_empty()) {
    return false;
  } else {
    return other.contains(top_left())
        || other.contains(top_right())
        || other.contains(bottom_left())
        || other.contains(top_right());
  }
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

BoundingBox &BoundingBox::operator |=(const Vec2f &point)
{
  *this = *this | point;
  return *this;
}

BoundingBox BoundingBox::around_selected_objects(const Scene &scene)
{
  BoundingBox bb;
  for (const auto* o : scene.item_selection<Object>()) {
    bb |= o->recursive_bounding_box(o->global_transformation(false));
  }
  return bb;
}

BoundingBox operator|(const BoundingBox& a, const BoundingBox& b)
{
  if (a.is_empty()) {
    return b;
  } else if (b.is_empty()) {
    return a;
  } else {
    return BoundingBox({ b.top_left(), b.bottom_right(), a.top_left(), a.bottom_right() });
  }
}

BoundingBox operator|(const BoundingBox &a, const Vec2f &b)
{
  if (a.is_empty()) {
    return BoundingBox({ b });
  } else {
    const double left = std::min(a.left(), b.x);
    const double right = std::max(a.right(), b.x);
    const double top = std::min(a.top(), b.y);
    const double bottom = std::max(a.bottom(), b.y);

    return BoundingBox(Vec2f(left, top), Vec2f(right, bottom));
  }
}

}  // namespace omm
