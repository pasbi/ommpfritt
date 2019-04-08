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
  std::string type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Text");
  std::unique_ptr<Object> clone() const override;
  BoundingBox bounding_box() override;
  Flag flags() const override;
  static constexpr auto TEXT_PROPERTY_KEY = "text";
  void draw_object(AbstractRenderer& renderer, const Style& style) override;

  static constexpr auto WIDTH_PROPERTY_KEY = "width";

private:
  FontProperties m_font_properties;
  TextOptionProperties m_text_option_properties;
};

}  // namespace omm
