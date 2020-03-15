#pragma once

#include "tags/tag.h"
#include <Qt>

namespace omm
{
class ScriptTag : public Tag
{
public:
  explicit ScriptTag(Object& owner);
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "ScriptTag");
  static constexpr auto CODE_PROPERTY_KEY = "code";
  static constexpr auto UPDATE_MODE_PROPERTY_KEY = "update";
  static constexpr auto TRIGGER_UPDATE_PROPERTY_KEY = "trigger";
  void on_property_value_changed(Property* property) override;
  void evaluate() override;
  void force_evaluate() override;
  Flag flags() const override;
};

}  // namespace omm
