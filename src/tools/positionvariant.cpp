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
  for (auto&& object : scene.object_selection()) {
    const auto t = object->global_transformation();
    if (object->type() == Path::TYPE) {
      const auto points = static_cast<Path*>(object)->points();
      handles.reserve(handles.size() + points.size());
      for (auto&& point : points) {
        handles.push_back(std::make_unique<PointSelectHandle>(*point));
      }
    }
  }
}

void PointPositions::transform(const ObjectTransformation& transformation)
{

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

}  // namespace omm
