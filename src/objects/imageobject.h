#pragma once

#include "objects/object.h"
#include <Qt>

namespace omm
{

class ImageObject : public Object
{
public:
  explicit ImageObject(Scene* scene);
  void draw_object(Painter& renderer, const Style&) const override;
  std::string type() const override;
  BoundingBox bounding_box(const ObjectTransformation& transformation) const override;
  std::unique_ptr<Object> clone() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "ImageObject");
  static constexpr auto FILEPATH_PROPERTY_KEY = "filename";
  static constexpr auto WIDTH_PROPERTY_KEY = "width";
  static constexpr auto OPACITY_PROPERTY_KEY = "opacity";
  static constexpr auto PAGE_PROPERTY_KEY = "page";
  static constexpr auto HANCHOR_PROPERTY_KEY = "hanchor";
  static constexpr auto VANCHOR_PROPERTY_KEY = "vanchor";

private:
  QPointF pos(const QSizeF& size) const;
};

}  // namespace omm
