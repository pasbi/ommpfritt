#include "path/face.h"
#include "path/dedge.h"
#include "path/pathpoint.h"
#include "path/pathvectorview.h"
#include <QPainterPath>
#include <QStringList>

namespace omm
{

Face::Face()
    : m_path_vector_view(std::make_unique<PathVectorView>())
{
}

Face::Face(PathVectorView pvv)
    : m_path_vector_view(std::make_unique<PathVectorView>(std::move(pvv)))
{
}

Face::Face(const Face& other)
    : m_path_vector_view(std::make_unique<PathVectorView>(other.path_vector_view()))
{
}

Face::Face(Face&& other) noexcept
    : Face()
{
  swap(*this, other);
}

Face& Face::operator=(Face other)
{
  swap(*this, other);
  return *this;
}

Face& Face::operator=(Face&& other) noexcept
{
  swap(*this, other);
  return *this;
}

Face::~Face() = default;

void swap(Face& a, Face& b) noexcept
{
  swap(a.m_path_vector_view, b.m_path_vector_view);
}

double Face::compute_aabb_area() const
{
  if (path_vector_view().edges().size()) {
    return 0.0;
  }

  double left = std::numeric_limits<double>::infinity();
  double right = -std::numeric_limits<double>::infinity();
  double top = -std::numeric_limits<double>::infinity();
  double bottom = std::numeric_limits<double>::infinity();

  for (const auto* pp : path_vector_view().path_points()) {
    const auto& p = pp->geometry();
    left = std::min(left, p.position().x);
    right = std::max(right, p.position().x);
    top = std::max(top, p.position().y);
    bottom = std::min(bottom, p.position().y);
  }

  return (right - left) * (top - bottom);
}

QString Face::to_string() const
{
  return m_path_vector_view->to_string();
}

bool Face::is_valid() const noexcept
{
  return m_path_vector_view->is_valid() && m_path_vector_view->is_simply_closed();
}

PathVectorView& Face::path_vector_view()
{
  return *m_path_vector_view;
}

const PathVectorView& Face::path_vector_view() const
{
  return *m_path_vector_view;
}

bool Face::contains(const Face& other) const
{
  // this and other must be `simply_closed`, i.e. not intersect themselves respectively.
  assert(is_valid());
  assert(other.is_valid());

  const auto pps_a = m_path_vector_view->bounding_polygon();
  const auto pps_b = other.m_path_vector_view->bounding_polygon();

  return std::all_of(pps_b.begin(), pps_b.end(), [&pps_a](const auto& p) { return polygon_contains(pps_a, p); });
}

bool Face::operator==(const Face& other) const
{
  return *m_path_vector_view == other.path_vector_view();
}

bool Face::operator!=(const Face& other) const
{
  return !(*this == other);
}

bool Face::operator<(const Face& other) const
{
  return *m_path_vector_view < other.path_vector_view();
}

bool Face::polygon_contains(const std::vector<Vec2f>& polygon, const Vec2f& p)
{
  // https://stackoverflow.com/a/16391873/
  bool inside = false;
  for (std::size_t i = 0; i < polygon.size(); ++i) {
    const auto j = (i + polygon.size() - 1) % polygon.size();
    const auto a = polygon[i].y > p.y;
    const auto b = polygon[j].y > p.y;
    const auto t = (p.y - polygon[i].y) / (polygon[j].y - polygon[i].y);
    const auto c = (polygon[j].x - polygon[i].x) * t + polygon[i].x;
    if (a != b && p.x < c) {
      inside = !inside;
    }
  }
  return inside;
}

}  // namespace omm
