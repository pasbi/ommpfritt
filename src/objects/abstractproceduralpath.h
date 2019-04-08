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
  void draw_object(AbstractRenderer& renderer, const Style& style) override;
  BoundingBox bounding_box() override;
  std::unique_ptr<Object> convert() override;
  Flag flags() const override;

  Point evaluate(const double t) override;
  double path_length() override;

  std::vector<double> cut(const Vec2f& c_start, const Vec2f& c_end);

protected:
  virtual std::vector<Point> points() = 0;
  virtual bool is_closed() const = 0;
};

}  // namespace omm
