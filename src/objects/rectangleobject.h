#pragma once

#include "objects/abstractpath.h"

namespace omm
{

class RectangleObject : public AbstractPath
{
public:
  explicit RectangleObject(Scene* scene);
  QString type() const override;
  Flag flags() const override;
  std::vector<Point> points() const override;
  bool is_closed() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "RectangleObject");

protected:
  void on_property_value_changed(Property *property) override;

  static constexpr auto SIZE_PROPERTY_KEY = "size";
  static constexpr auto RADIUS_PROPERTY_KEY = "r";
  static constexpr auto TENSION_PROPERTY_KEY = "tension";
};

}  // namespace
