#pragma once

#include <armadillo>
#include "tools/handles/handle.h"

namespace omm
{

class Scene;
class Path;


class ObjectSelectHandle : public Handle
{
public:
  explicit ObjectSelectHandle(Scene& scene, Object& object);
  bool contains(const arma::vec2& point) const override;
  void draw(omm::AbstractRenderer& renderer) const override;
  bool mouse_press(const arma::vec2& pos) override;

protected:
  ObjectTransformation transformation() const override;

private:
  Scene& m_scene;
  Object& m_object;
};

class PointSelectHandle : public Handle
{
public:
  explicit PointSelectHandle(Path& path, Point& point);
  bool contains(const arma::vec2& point) const override;
  void draw(omm::AbstractRenderer& renderer) const override;
  bool mouse_press(const arma::vec2& pos) override;

protected:
  ObjectTransformation transformation() const override;

private:
  Path& m_path;
  Point& m_point;
};



}  // namespace omm
