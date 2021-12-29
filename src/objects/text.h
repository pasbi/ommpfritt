#pragma once

#include "objects/object.h"
#include "properties/propertygroups/fontproperties.h"
#include "properties/propertygroups/textoptionproperties.h"
#include <Qt>

namespace omm
{
class Scene;

class Text : public Object
{
public:
  explicit Text(Scene* scene);
  Text(const Text& other);
  Text(Text&&) = delete;
  Text& operator=(Text&&) = delete;
  Text& operator=(const Text&) = delete;
  ~Text() override = default;

  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Text");
  BoundingBox bounding_box(const ObjectTransformation& transformation) const override;
  Flag flags() const override;
  static constexpr auto TEXT_PROPERTY_KEY = "text";
  void draw_object(Painter& renderer,
                   const Style& style,
                   const PainterOptions& options) const override;

  static constexpr auto WIDTH_PROPERTY_KEY = "width";
  QRectF rect(Qt::Alignment alignment) const;

protected:
  void on_property_value_changed(Property* property) override;
  PathVector compute_path_vector() const override;

private:
  FontProperties m_font_properties;
  TextOptionProperties m_text_option_properties;
};

}  // namespace omm
