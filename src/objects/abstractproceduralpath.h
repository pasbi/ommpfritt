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
  void render(AbstractRenderer& renderer, const Style& style) override;
  BoundingBox bounding_box() override;
  static constexpr auto TYPE
    = QT_TRANSLATE_NOOP("AbstractProceduralPath", "AbstractProceduralPath");
  std::unique_ptr<Object> convert() override;
  Flag flags() const override;

  OrientedPoint evaluate(const double t) override;
  double path_length() override;

protected:
  virtual std::vector<Point> points() = 0;
  virtual bool is_closed() const = 0;
};

}  // namespace omm
