#pragma once

#include <armadillo>
#include <set>
#include <vector>
#include "tools/handles/selecthandle.h"

namespace arma
{
bool operator<(const arma::vec2& a, const arma::vec2& b);
}  // namespace arma

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
  void make_handles(handles_type& handles, Tool& tool) const;
  void clear_selection();
  arma::vec2 selection_center() const;
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
  arma::vec2 selection_center() const;
  double selection_rotation() const;
  bool is_empty() const;

};

}  // namespace omm

