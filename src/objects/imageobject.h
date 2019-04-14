#pragma once

#include "objects/object.h"
#include <Qt>

namespace omm
{

class ImageObject : public Object
{
public:
  explicit ImageObject(Scene* scene);
  void draw_object(AbstractRenderer& renderer, const Style&) const override;
  std::string type() const override;
  BoundingBox bounding_box() const override;
  std::unique_ptr<Object> clone() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "ImageObject");
  static constexpr auto FILEPATH_PROPERTY_KEY = "filename";
  static constexpr auto WIDTH_PROPERTY_KEY = "width";
  static constexpr auto OPACITY_PROPERTY_KEY = "opacity";
};

}  // namespace omm
