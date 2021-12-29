#pragma once

#include "objects/object.h"

namespace omm
{
class LineObject : public Object
{
public:
  explicit LineObject(Scene* scene);
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "LineObject");
  Flag flags() const override;

  static constexpr auto LENGTH_PROPERTY_KEY = "length";
  static constexpr auto ANGLE_PROPERTY_KEY = "angle";
  static constexpr auto CENTER_PROPERTY_KEY = "center";

  PathVector compute_path_vector() const override;

protected:
  void on_property_value_changed(Property* property) override;
};

}  // namespace omm
