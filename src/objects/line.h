#pragma once
#include "objects/object.h"

namespace omm
{
class Line : public Object
{
public:
  explicit Line(Scene* scene);
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Line");
  Flag flags() const override;

  static constexpr auto LENGTH_PROPERTY_KEY = "length";
  static constexpr auto ANGLE_PROPERTY_KEY = "angle";
  static constexpr auto CENTER_PROPERTY_KEY = "center";

  Geom::PathVector paths() const override;
  bool is_closed() const override
  {
    return false;
  }

protected:
  void on_property_value_changed(Property* property) override;
};

}  // namespace omm
