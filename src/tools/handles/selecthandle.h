#pragma once

#include <armadillo>
#include "tools/handles/handle.h"

namespace omm
{


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
  Object& m_object;
  Scene& m_scene;
};

class PointSelectHandle : public Handle
{
public:
  explicit PointSelectHandle(Point& point);
  bool contains(const arma::vec2& point) const override;
  void draw(omm::AbstractRenderer& renderer) const override;
private:
  Point& m_point;
};



}  // namespace omm
