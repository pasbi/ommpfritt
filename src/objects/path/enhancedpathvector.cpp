#include "objects/path/enhancedpathvector.h"
#include "objects/path.h"
#include <QPainterPath>

namespace
{

QPointF qpoint(const Geom::Point& vec)
{
  return {vec[0], vec[1]};
}

}  // namespace

namespace omm
{

EnhancedPathVector::EnhancedPathVector() = default;

EnhancedPathVector::EnhancedPathVector(const EnhancedPathVector& other)
  : m_path_vector(other.m_path_vector)
  , m_joined_points(other.m_joined_points)
  , m_outline(copy_unique_ptr(other.m_outline))
  , m_fill(copy_unique_ptr(other.m_fill))
{
}

EnhancedPathVector::EnhancedPathVector(EnhancedPathVector&& other) noexcept
{
  swap(*this, other);
}

EnhancedPathVector& EnhancedPathVector::operator=(const EnhancedPathVector& other)
{
  auto copy{other};
  swap(*this, copy);
  return *this;
}

EnhancedPathVector& EnhancedPathVector::operator=(EnhancedPathVector&& other) noexcept
{
  swap(*this, other);
  return *this;
}

EnhancedPathVector::EnhancedPathVector(const Geom::PathVector& path_vector, const JoinedPointIndices& joined_points)
  : m_path_vector(path_vector)
  , m_joined_points(joined_points)
{
}

const Geom::PathVector& EnhancedPathVector::path_vector() const
{
  return m_path_vector;
}

const QPainterPath& EnhancedPathVector::fill() const
{
  if (!m_fill) {
    m_fill = std::make_unique<QPainterPath>();
  }
  return *m_fill;
}

const QPainterPath& EnhancedPathVector::outline() const
{
  if (!m_outline) {
    m_outline = std::make_unique<QPainterPath>();
    for (const Geom::Path& path : m_path_vector) {
      m_outline->moveTo(qpoint(path.initialPoint()));
      for (const Geom::Curve& curve : path) {
        const auto& cbc = dynamic_cast<const Geom::CubicBezier&>(curve);
        m_outline->cubicTo(qpoint(cbc[1]), qpoint(cbc[2]), qpoint(cbc[3]));
      }
    }
  }
  return *m_outline;
}

DisjointPathPointSetForest EnhancedPathVector::joined_points(const Path& path) const
{
  DisjointPathPointSetForest forest;
  for (const auto& set : m_joined_points.sets()) {
    std::set<PathPoint*> joined_points;
    for (const auto& index : set) {
      joined_points.insert(&path.point_at_index(index));
    }
    forest.insert(joined_points);
  }
  return forest;
}

void swap(EnhancedPathVector& a, EnhancedPathVector& b) noexcept
{
  auto apv = std::move(a.m_path_vector);
  a.m_path_vector = std::move(b.m_path_vector);
  b.m_path_vector = std::move(apv);

  swap(a.m_joined_points, b.m_joined_points);
  swap(a.m_fill, b.m_fill);
  swap(a.m_outline, b.m_outline);
}

}  // namespace omm
