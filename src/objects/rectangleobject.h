#pragma once

#include "objects/object.h"

namespace omm
{
class RectangleObject : public Object
{
public:
  explicit RectangleObject(Scene* scene);
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "RectangleObject");
  EnhancedPathVector paths() const override;

protected:
  void on_property_value_changed(Property* property) override;

  static constexpr auto SIZE_PROPERTY_KEY = "size";
  static constexpr auto RADIUS_PROPERTY_KEY = "r";
  static constexpr auto TENSION_PROPERTY_KEY = "tension";
};

}  // namespace omm
