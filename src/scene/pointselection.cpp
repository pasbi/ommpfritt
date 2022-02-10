#include "scene/pointselection.h"
#include "objects/pathobject.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "scene/scene.h"

namespace omm
{

PointSelection::PointSelection(Scene& scene) : m_scene(scene)
{
}

::transparent_set<PathPoint*> PointSelection::points() const
{
  ::transparent_set<PathPoint*> selected_points;
  for (auto* path_object : type_casts<PathObject*>(m_scene.item_selection<Object>())) {
    for (auto* point : path_object->geometry().selected_points()) {
      selected_points.insert(point);
    }
  }
  return selected_points;
}

std::set<Point> PointSelection::points(Space space) const
{
  std::set<Point> selected_points;
  for (auto* pp : points()) {
    const auto transformation = pp->path_vector()->path_object()->global_transformation(space);
    selected_points.insert(transformation.apply(pp->geometry()));
  }
  return selected_points;
}

Vec2f PointSelection::center(Space space) const
{
  const auto selected_points = points(space);
  Vec2f sum(0.0, 0.0);
  for (const Point& p : selected_points) {
    sum += p.position();
  }
  return sum / static_cast<double>(selected_points.size());
}

}  // namespace omm
