#pragma once

#include "objects/object.h"
#include <Qt>

namespace omm
{

class Scene;

class AbstractProceduralPath : public Object
{
public:
  using Object::Object;
  void draw_object(Painter& renderer, const Style& style) const override;
  BoundingBox bounding_box() const override;
  std::unique_ptr<Object> convert() const override;
  Flag flags() const override;

  Point evaluate(const double t) const override;
  double path_length() const override;
  bool contains(const Vec2f &pos) const override;

  std::vector<double> cut(const Vec2f& c_start, const Vec2f& c_end);
  PathUniquePtr outline(const double t) const override;

  void update() override;

private:
  std::vector<Point> m_points;
  QPainterPath m_painter_path;
};

}  // namespace omm
