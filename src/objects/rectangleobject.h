#pragma once

#include "objects/abstractproceduralpath.h"

namespace omm
{

class RectangleObject : public AbstractProceduralPath
{
public:
  explicit RectangleObject(Scene* scene);
  QString type() const override;
  std::unique_ptr<Object> clone() const override;
  Flag flags() const override;
  std::vector<Point> points() const override;
  bool is_closed() const override;

protected:
  void on_property_value_changed(Property *property) override;

  static constexpr auto SIZE_PROPERTY_KEY = "size";
  static constexpr auto RADIUS_PROPERTY_KEY = "r";
  static constexpr auto TENSION_PROPERTY_KEY = "tension";
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "RectangleObject");
};

}  // namespace
