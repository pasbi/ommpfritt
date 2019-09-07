#pragma once

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
  std::set<Point> points(bool skip_root) const;
  Vec2f center(bool skip_root) const;

private:
  Scene& m_scene;
};

}  // namespace
