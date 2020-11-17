#pragma once

#include "tags/tag.h"
#include <Qt>
#include <memory>

namespace omm
{
class StyleTag : public Tag
{
public:
  explicit StyleTag(Object& owner);
  QString type() const override;
  static constexpr auto STYLE_REFERENCE_PROPERTY_KEY = "style";
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "StyleTag");
  static constexpr auto EDIT_STYLE_PROPERTY_KEY = "edit-style";
  void evaluate() override;
  Flag flags() const override;

protected:
  void on_property_value_changed(Property* property) override;
};

}  // namespace omm
