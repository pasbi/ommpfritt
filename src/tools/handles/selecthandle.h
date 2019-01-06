#pragma once

#include <armadillo>
#include "tools/handles/handle.h"

namespace omm
{

class Scene;
class Path;

template<typename PositionVariant> class SelectTool;
class PointPositions;
class ObjectPositions;

class ObjectSelectHandle : public Handle
{
public:
  explicit ObjectSelectHandle(SelectTool<ObjectPositions>& tool, Scene& scene, Object& object);
  bool contains(const arma::vec2& point) const override;
  void draw(omm::AbstractRenderer& renderer) const override;
  bool mouse_press(const arma::vec2& pos) override;
  void mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover) override;

protected:
  ObjectTransformation transformation() const override;

private:
  SelectTool<ObjectPositions>& m_tool;
  Scene& m_scene;
  Object& m_object;
};

class PointSelectHandle : public Handle
{
public:
  explicit PointSelectHandle(SelectTool<PointPositions>& tool, Path& path, Point& point);
  bool contains(const arma::vec2& point) const override;
  void draw(omm::AbstractRenderer& renderer) const override;
  bool mouse_press(const arma::vec2& pos) override;
  void mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover) override;

protected:
  ObjectTransformation transformation() const override;

private:
  SelectTool<PointPositions>& m_tool;
  Path& m_path;
  Point& m_point;
};



}  // namespace omm
