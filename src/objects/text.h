#pragma once

#include "objects/object.h"
#include <Qt>
#include "properties/propertygroups/fontproperties.h"
#include "properties/propertygroups/textoptionproperties.h"

namespace omm
{

class Scene;

class Text : public Object
{
public:
  explicit Text(Scene* scene);
  Text(const Text& other);
  std::string type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Text");
  std::unique_ptr<Object> clone() const override;
  BoundingBox bounding_box() const override;
  Flag flags() const override;
  static constexpr auto TEXT_PROPERTY_KEY = "text";
  void draw_object(Painter& renderer, const Style& style) const override;

  static constexpr auto WIDTH_PROPERTY_KEY = "width";

private:
  FontProperties m_font_properties;
  TextOptionProperties m_text_option_properties;
};

}  // namespace omm
