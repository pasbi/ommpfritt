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

template<typename PositionRepr>
class AbstractPositions
{
public:
  AbstractPositions(Scene& scene) : scene(scene) {};
  virtual ~AbstractPositions() = default;

protected:
  Scene& scene;
};

class PointPositions : public AbstractPositions<Point>
{
public:
  using AbstractPositions<Point>::AbstractPositions;
  void make_handles(std::vector<std::unique_ptr<Handle>>& handles) const;
  void transform(const ObjectTransformation& transformation);
  void clear_selection();
  arma::vec2 selection_center() const;
  double selection_rotation() const;
  bool is_empty() const;

private:
  std::set<Point*> selected_points() const;
  std::set<Path*> paths() const;
};

class ObjectPositions : public AbstractPositions<Object>
{
public:
  using AbstractPositions<Object>::AbstractPositions;
  void make_handles(std::vector<std::unique_ptr<Handle>>& handles) const;
  void transform(const ObjectTransformation& transformation);
  void clear_selection();
  arma::vec2 selection_center() const;
  double selection_rotation() const;
  bool is_empty() const;
};

}  // namespace omm

