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
  std::set<Point> points() const;
  Vec2f center() const;

private:
  Scene& m_scene;
};

}  // namespace
