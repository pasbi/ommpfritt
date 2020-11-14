#pragma once

#include "common.h"
#include "geometry/point.h"
#include <set>

namespace omm
{
class Path;
class Scene;

class PointSelection
{
public:
  PointSelection(Scene& scene);
  std::set<Point> points(Space space) const;
  Vec2f center(Space space) const;

private:
  Scene& m_scene;
};

}  // namespace omm
