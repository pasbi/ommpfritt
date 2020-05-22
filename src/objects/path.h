#pragma once

#include "objects/object.h"
#include "geometry/point.h"
#include <list>
#include "geometry/cubics.h"

namespace omm
{

class Scene;

class Path : public Object
{
public:
  explicit Path(Scene* scene);
  void draw_object(Painter& renderer, const Style& style, Painter::Options options) const override;
  BoundingBox bounding_box(const ObjectTransformation& transformation) const override;
  QString type() const override;

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Path");
  static constexpr auto IS_CLOSED_PROPERTY_KEY = "closed";
  static constexpr auto POINTS_POINTER = "points";
  static constexpr auto INTERPOLATION_PROPERTY_KEY = "interpolation";

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;

  enum class InterpolationMode { Linear, Smooth, Bezier };
  Flag flags() const override;

  void update() override;

private:

};

}  // namespace omm
