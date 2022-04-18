#pragma once

#include <vector>
#include "path/pathgeometry.h"

class QPainterPath;

namespace omm
{

class PathGeometry;

class PathVectorGeometry
{
public:
  explicit PathVectorGeometry(std::vector<PathGeometry> paths);
  PathVectorGeometry() noexcept;
  ~PathVectorGeometry();
  const std::vector<PathGeometry>& paths() const;
  QPainterPath to_painter_path() const;

private:
  std::vector<PathGeometry> m_paths;
};

}  // namespace omm
