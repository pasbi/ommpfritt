#pragma once

#include "tags/tag.h"
#include <Qt>

namespace omm
{
class ScriptTag : public Tag
{
public:
  explicit ScriptTag(Object& owner);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = QT_TR_NOOP("ScriptTag");
  static constexpr auto CODE_PROPERTY_KEY = "code";
  static constexpr auto UPDATE_MODE_PROPERTY_KEY = "update";
  static constexpr auto TRIGGER_UPDATE_PROPERTY_KEY = "trigger";
  std::unique_ptr<Tag> clone() const override;
  void on_property_value_changed(Property& property) override;
  void evaluate() override;
  void force_evaluate();
  Flag flags() const override;
};

}  // namespace omm
