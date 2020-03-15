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
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Text");
  BoundingBox bounding_box(const ObjectTransformation& transformation) const override;
  Flag flags() const override;
  static constexpr auto TEXT_PROPERTY_KEY = "text";
  void draw_object(Painter& renderer, const Style& style, Painter::Options options) const override;

  static constexpr auto WIDTH_PROPERTY_KEY = "width";
  QRectF rect(Qt::Alignment alignment) const;

protected:
  void on_property_value_changed(Property *property) override;

private:
  FontProperties m_font_properties;
  TextOptionProperties m_text_option_properties;
};

}  // namespace omm
