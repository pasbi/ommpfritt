#include "tools/itemtools/positionvariant.h"
#include "objects/path.h"
#include "scene/scene.h"
#include "commands/objectstransformationcommand.h"
#include "commands/pointstransformationcommand.h"

bool arma::operator<(const arma::vec2& a, const arma::vec2& b)
{
  return a[0] == b[0] ? a[1] < b[1] : a[0] < b[0];
}

namespace omm
{

void PointPositions::make_handles(handles_type& handles, SelectTool<PointPositions>& tool) const
{
  for (auto* path : paths()) {
    const auto t = path->global_transformation();
    handles.reserve(handles.size() + path->points().size());
    for (auto&& point : path->points()) {
      handles.push_back(std::make_unique<PointSelectHandle>(tool, *path, *point));
    }
  }
}

void PointPositions::transform(const ObjectTransformation& transformation)
{
  scene.submit<PointsTransformationCommand>(paths(), transformation);
}

void PointPositions::clear_selection()
{
  for (auto* path : paths()) {
    for (auto* point : path->points()) {
      point->is_selected = false;
    }
  }
}

arma::vec2 PointPositions::selection_center() const
{
  std::set<arma::vec2> positions;
  for (auto* path : paths()) {
    for (auto* point : path->points()) {
      if (point->is_selected) {
        positions.insert(path->global_transformation().apply_to_position(point->position));
      }
    }
  }
  const auto null = arma::vec2 {0.0, 0.0};
  if (positions.size() > 0) {
    return std::accumulate(positions.begin(), positions.end(), null) / positions.size();
  } else {
    return null;
  }
}

double PointPositions::selection_rotation() const
{
  return 0.0;
}

std::set<Point*> PointPositions::selected_points() const
{
  std::set<Point*> selected_points;
  for (auto* path : paths()) {
    for (auto* point : path->points()) {
      if (point->is_selected) {
        selected_points.insert(point);
      }
    }
  }
  return selected_points;
}

bool PointPositions::is_empty() const
{
  return selected_points().size() == 0;
}

std::set<Path*> PointPositions::paths() const
{
  const auto is_path = [](const auto* o) { return o->type() == Path::TYPE; };
  const auto to_path = [](auto* o) { return static_cast<Path*>(o); };
  return ::transform<Path*>(::filter_if(scene.object_selection(), is_path), to_path);
}

void ObjectPositions::make_handles(handles_type& handles, SelectTool<ObjectPositions>& tool) const
{
  // ignore object selection. Return a handle for each object.
  const auto objects = scene.object_tree.items();
  handles.reserve(objects.size());
  auto inserter = std::back_inserter(handles);
  std::transform(objects.begin(), objects.end(), inserter, [this, &tool](Object* o) {
    return std::make_unique<ObjectSelectHandle>(tool, scene, *o);
  });
}

void ObjectPositions::transform(const ObjectTransformation& transformation)
{
  scene.submit<ObjectsTransformationCommand>(scene.object_selection(), transformation);
}

void ObjectPositions::clear_selection()
{
  scene.set_selection({});
}

arma::vec2 ObjectPositions::selection_center() const
{
  const auto objects = scene.object_selection();
  assert(objects.size() > 0);
  const auto add = [](const arma::vec2& accu, const Object* object) -> arma::vec2 {
    return accu + object->global_transformation().translation();
  };
  const auto null = arma::vec2 {0.0, 0.0};
  return std::accumulate(objects.begin(), objects.end(), null, add) / objects.size();
}

double ObjectPositions::selection_rotation() const
{
  const auto objects = scene.object_selection();
  if (objects.size() == 1) {
    return (**objects.begin()).global_transformation().rotation();
  } else {
    return 0.0;
  }
}

bool ObjectPositions::is_empty() const
{
  return scene.object_selection().size() == 0;
}

}  // namespace omm
