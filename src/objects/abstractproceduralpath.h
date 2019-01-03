#pragma once

#include "objects/object.h"

namespace omm
{

class Scene;

class AbstractProceduralPath : public Object
{
public:
  using Object::Object;
  void render(AbstractRenderer& renderer, const Style& style) override;
  BoundingBox bounding_box() const override;
  static constexpr auto TYPE = "AbstractProceduralPath";
  std::unique_ptr<Object> convert() const override;
  Object::Flag flags() const override;

protected:
  virtual std::vector<Point> points() const = 0;
  virtual bool is_closed() const = 0;
};

}  // namespace omm
