#include "path/pathvectorgeometry.h"
#include "path/pathgeometry.h"
#include "geometry/point.h"
#include <QPainterPath>

namespace omm
{

PathVectorGeometry::PathVectorGeometry(std::vector<PathGeometry> paths)
    : m_paths(std::move(paths))
{
}

PathVectorGeometry::PathVectorGeometry() noexcept = default;

PathVectorGeometry::~PathVectorGeometry() = default;

const std::vector<PathGeometry>& PathVectorGeometry::paths() const
{
  return m_paths;
}

QPainterPath PathVectorGeometry::to_painter_path() const
{
  QPainterPath outline;
  for (const PathGeometry& path : paths()) {
    outline.addPath(path.to_painter_path());
  }
  return outline;
}

}  // namespace omm
