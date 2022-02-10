#include <cassert>

#include "common.h"
#include "geometry/boundingbox.h"
#include "scene/scene.h"
#include <algorithm>

namespace
{
double min(const std::set<double>& ds)
{
  if (ds.empty()) {
    return 0.0;
  } else {
    return *std::min_element(ds.begin(), ds.end());
  }
}
double max(const std::set<double>& ds)
{
  if (ds.empty()) {
    return 0.0;
  } else {
    return *std::max_element(ds.begin(), ds.end());
  }
}

std::set<omm::Vec2f> get_all_control_points(const std::set<omm::Point>& points)
{
  std::set<omm::Vec2f> control_points;
  for (auto&& p : points) {
    control_points.insert(p.left_position());
    control_points.insert(p.position());
    control_points.insert(p.right_position());
  }
  return control_points;
}

std::set<omm::Vec2f> get_all_points(const std::set<omm::BoundingBox>& bbs)
{
  std::set<omm::Vec2f> points;
  for (const omm::BoundingBox& bb : bbs) {
    points.insert(bb.top_left());
    points.insert(bb.top_right());
    points.insert(bb.bottom_left());
    points.insert(bb.bottom_right());
  }
  return points;
}

}  // namespace

namespace omm
{
BoundingBox::BoundingBox(const std::set<Vec2f>& points)
    : BoundingBox(util::transform(points, [](const Vec2f& v) { return v.x; }),
                  util::transform(points, [](const Vec2f& v) { return v.y; }))
{
}

BoundingBox::BoundingBox(const std::set<double>& xs, const std::set<double>& ys)
    : Rectangle(Vec2f(min(xs), min(ys)), Vec2f(max(xs), max(ys)), xs.empty())
{
  assert(xs.empty() == ys.empty());
}

BoundingBox::BoundingBox(const std::set<Point>& points)
    : BoundingBox(get_all_control_points(points))
{
}

BoundingBox::BoundingBox(const std::set<BoundingBox>& bbs) : BoundingBox(get_all_points(bbs))
{
}

bool BoundingBox::contains(const BoundingBox& other) const
{
  if (is_empty()) {
    return false;
  } else {
    return other.contains(top_left()) || other.contains(top_right())
           || other.contains(bottom_left()) || other.contains(top_right());
  }
}

QString BoundingBox::to_string() const
{
  return QString{"BoundingBox[%1, %2]"}.arg(top_left().to_string(), bottom_right().to_string());
}

BoundingBox& BoundingBox::operator|=(const BoundingBox& other)
{
  *this = *this | other;
  return *this;
}

BoundingBox& BoundingBox::operator|=(const Vec2f& point)
{
  *this = *this | point;
  return *this;
}

BoundingBox BoundingBox::around_selected_objects(const Scene& scene)
{
  BoundingBox bb;
  for (const auto* o : scene.item_selection<Object>()) {
    bb |= o->recursive_bounding_box(o->global_transformation(Space::Viewport));
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
    return BoundingBox({b.top_left(), b.bottom_right(), a.top_left(), a.bottom_right()});
  }
}

BoundingBox operator|(const BoundingBox& a, const Vec2f& b)
{
  if (a.is_empty()) {
    return BoundingBox({b});
  } else {
    const double left = std::min(a.left(), b.x);
    const double right = std::max(a.right(), b.x);
    const double top = std::min(a.top(), b.y);
    const double bottom = std::max(a.bottom(), b.y);

    return BoundingBox(Vec2f(left, top), Vec2f(right, bottom));
  }
}

bool operator<(const BoundingBox& a, const BoundingBox& b)
{
  if (a.top_left() != b.top_left()) {
    return a.top_left() < b.top_left();
  } else {
    return a.top_right() < b.top_right();
  }
}

}  // namespace omm
