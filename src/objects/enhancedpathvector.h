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
  EnhancedPathVector();
  EnhancedPathVector(const EnhancedPathVector& other);
  EnhancedPathVector(EnhancedPathVector&& other) noexcept;
  EnhancedPathVector& operator=(const EnhancedPathVector& other);
  EnhancedPathVector& operator=(EnhancedPathVector&& other) noexcept;
  EnhancedPathVector(const Geom::PathVector& path_vector, const DisjointSetForest<std::size_t>& joined_points = {});
  const Geom::PathVector& path_vector() const;
  const QPainterPath& qpainter_path() const;
  friend void swap(EnhancedPathVector& a, EnhancedPathVector& b) noexcept;

private:
  Geom::PathVector m_path_vector;
  DisjointSetForest<std::size_t> m_joined_points;
  mutable std::unique_ptr<QPainterPath> m_qpainter_path;
};

}  // namespace omm