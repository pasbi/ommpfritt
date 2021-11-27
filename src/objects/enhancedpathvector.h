#pragma once

#include "disjointset.h"
#include "scene/disjointpathpointsetforest.h"
#include <2geom/2geom.h>
#include <memory>

class QPainterPath;

namespace omm
{

class Path;

class EnhancedPathVector
{
public:
  using JoinedPointIndices =  DisjointSetForest<std::size_t>;
  EnhancedPathVector();
  EnhancedPathVector(const EnhancedPathVector& other);
  EnhancedPathVector(EnhancedPathVector&& other) noexcept;
  EnhancedPathVector& operator=(const EnhancedPathVector& other);
  EnhancedPathVector& operator=(EnhancedPathVector&& other) noexcept;
  EnhancedPathVector(const Geom::PathVector& path_vector, const JoinedPointIndices& joined_points = {});
  const Geom::PathVector& path_vector() const;
  const QPainterPath& qpainter_path() const;
  friend void swap(EnhancedPathVector& a, EnhancedPathVector& b) noexcept;
  DisjointPathPointSetForest joined_points(const Path& path) const;

private:
  Geom::PathVector m_path_vector;
  JoinedPointIndices m_joined_points;
  mutable std::unique_ptr<QPainterPath> m_qpainter_path;
};

}  // namespace omm
