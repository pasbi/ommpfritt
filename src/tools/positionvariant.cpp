#include "tools/positionvariant.h"
#include "objects/path.h"
#include "scene/scene.h"

namespace
{

template<typename Ts, typename T, typename... F> T mean(const Ts& ts, const T& null, F&&... fs)
{
  if (ts.size() > 0) {
    return std::accumulate(std::begin(ts), std::end(ts), null, fs...)
            / static_cast<double>(ts.size());
  } else {
    return null;
  }
}

}

namespace omm
{

void PointPositions::make_handles(handles_type& handles, Tool& tool) const
{
  for (auto* path : paths()) {
    handles.reserve(handles.size() + path->points().size());
    for (auto* point : path->points_ref()) {
      handles.push_back(std::make_unique<PointSelectHandle>(tool, *path, *point));
    }
  }
}

void PointPositions::clear_selection()
{
  for (auto* path : paths()) {
    for (auto* point : path->points_ref()) {
      point->is_selected = false;
    }
  }
}

Vec2f PointPositions::selection_center() const
{
  std::set<Vec2f> positions;
  for (auto* path : paths()) {
    for (auto* point : path->points_ref()) {
      if (point->is_selected) {
        positions.insert(path->global_transformation().apply_to_position(point->position));
      }
    }
  }
  return mean(positions, Vec2f::o());
}

double PointPositions::selection_rotation() const
{
  return 0.0;
}

std::set<Point*> PointPositions::selected_points() const
{
  std::set<Point*> selected_points;
  for (auto* path : paths()) {
    for (auto* point : path->points_ref()) {
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
  return ::transform<Path*>(::filter_if(scene.item_selection<Object>(), is_path), to_path);
}

void ObjectPositions::make_handles(handles_type& handles, Tool& tool) const
{
  // ignore object selection. Return a handle for each visible object.
  const auto objects = ::filter_if(scene.object_tree.items(), [](Object* object) {
    return object->is_visible();
  });

  handles.reserve(objects.size());
  auto inserter = std::back_inserter(handles);
  std::transform(objects.begin(), objects.end(), inserter, [this, &tool](Object* o) {
    return std::make_unique<ObjectSelectHandle>(tool, scene, *o);
  });
}

void ObjectPositions::clear_selection()
{
  scene.set_selection({});
}

Vec2f ObjectPositions::selection_center() const
{
  const auto objects = scene.item_selection<Object>();
  const auto add = [](const Vec2f& accu, const Object* object) -> Vec2f {
    return accu + object->global_transformation().translation();
  };
  return mean(objects, Vec2f::o(), add);
}

double ObjectPositions::selection_rotation() const
{
  const auto objects = scene.item_selection<Object>();
  if (objects.size() == 1) {
    return (**objects.begin()).global_transformation().rotation();
  } else {
    return 0.0;
  }
}

bool ObjectPositions::is_empty() const
{
  return scene.item_selection<Object>().size() == 0;
}

}  // namespace omm
