#include "tools/positionvariant.h"
#include "objects/path.h"
#include "scene/scene.h"

bool arma::operator<(const arma::vec2& a, const arma::vec2& b)
{
  return a[0] == b[0] ? a[1] < b[1] : a[0] < b[0];
}

namespace omm
{

void PointPositions::make_handles(std::vector<std::unique_ptr<Handle>>& handles) const
{
  for (auto* path : paths()) {
    const auto t = path->global_transformation();
    handles.reserve(handles.size() + path->points().size());
    for (auto&& point : path->points()) {
      handles.push_back(std::make_unique<PointSelectHandle>(*path, *point));
    }
  }
}

void PointPositions::transform(const ObjectTransformation& transformation)
{

}

void PointPositions::clear_selection()
{
  for (auto* path : paths()) {
    for (auto* point : path->points()) {
      point->is_selected = false;
    }
  }
}

std::set<Path*> PointPositions::paths() const
{
  const auto is_path = [](const auto* o) { return o->type() == Path::TYPE; };
  const auto to_path = [](auto* o) { return static_cast<Path*>(o); };
  return ::transform<Path*>(::filter_if(scene.object_selection(), is_path), to_path);
}

void ObjectPositions::make_handles(std::vector<std::unique_ptr<Handle>>& handles) const
{
  // ignore object selection. Return a handle for each object.
  const auto objects = scene.object_tree.items();
  handles.reserve(objects.size());
  auto inserter = std::back_inserter(handles);
  std::transform(objects.begin(), objects.end(), inserter, [this](Object* o) {
    return std::make_unique<ObjectSelectHandle>(scene, *o);
  });
}

void ObjectPositions::transform(const ObjectTransformation& transformation)
{

}

void ObjectPositions::clear_selection()
{
  scene.set_selection({});
}

}  // namespace omm
