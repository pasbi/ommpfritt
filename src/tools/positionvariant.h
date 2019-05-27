#pragma once

#include "geometry/vec2.h"
#include <set>
#include <vector>
#include "tools/handles/selecthandle.h"

namespace omm
{

class Object;
class ObjectTransformation;
class Point;
class Scene;
template<typename ItemT> class ItemTool;

template<typename PositionRepr>
class AbstractPositions
{
public:
  using handles_type = std::vector<std::unique_ptr<Handle>>;
  AbstractPositions(Scene& scene) : scene(scene) {};
  virtual ~AbstractPositions() = default;

protected:
  Scene& scene;
};

class PointPositions : public AbstractPositions<Point>
{
public:
  using AbstractPositions<Point>::AbstractPositions;
  void make_handles(handles_type& handles, Tool& tool, bool force_draw_subhandle = false) const;
  void clear_selection();
  Vec2f selection_center() const;
  double selection_rotation() const;
  bool is_empty() const;
  std::set<Path*> paths() const;

private:
  std::set<Point*> selected_points() const;
};

class ObjectPositions : public AbstractPositions<Object>
{
public:
  using AbstractPositions<Object>::AbstractPositions;
  void make_handles(handles_type& handles, Tool& tool) const;
  void clear_selection();
  Vec2f selection_center() const;
  double selection_rotation() const;
  bool is_empty() const;

};

}  // namespace omm

