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
};

class ObjectPositions : public AbstractPositions<Object>
{
public:
  using AbstractPositions<Object>::AbstractPositions;
  void make_handles(std::vector<std::unique_ptr<Handle>>& handles) const;
  void transform(const ObjectTransformation& transformation);
};

}  // namespace omm

