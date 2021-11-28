#include "objects/path/enhancedpathvector.h"
#include "objects/path.h"
#include <QPainterPath>

namespace omm
{

EnhancedPathVector::EnhancedPathVector() = default;

EnhancedPathVector::EnhancedPathVector(const EnhancedPathVector& other)
  : m_path_vector(other.m_path_vector)
  , m_joined_points(other.m_joined_points)
  , m_qpainter_path(other.m_qpainter_path ? std::make_unique<QPainterPath>(*other.m_qpainter_path) : nullptr)
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

const QPainterPath& EnhancedPathVector::qpainter_path() const
{
  if (!m_qpainter_path) {
    static const auto qpoint = [](const Geom::Point& point) { return QPointF{point[0], point[1]}; };
    m_qpainter_path = std::make_unique<QPainterPath>();
    for (const Geom::Path& path : m_path_vector) {
      m_qpainter_path->moveTo(qpoint(path.initialPoint()));
      for (const Geom::Curve& curve : path) {
        const auto& cbc = dynamic_cast<const Geom::CubicBezier&>(curve);
        m_qpainter_path->cubicTo(qpoint(cbc[1]), qpoint(cbc[2]), qpoint(cbc[3]));
      }
    }
  }
  return *m_qpainter_path;
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
  swap(a.m_qpainter_path, b.m_qpainter_path);
}

}  // namespace omm
