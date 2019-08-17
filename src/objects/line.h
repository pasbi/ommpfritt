#pragma once
#include "objects/abstractproceduralpath.h"

namespace omm
{

class Line : public AbstractProceduralPath
{
public:
  explicit Line(Scene* scene);
  std::string type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Line");
  std::unique_ptr<Object> clone() const override;
  Flag flags() const override;

  static constexpr auto LENGTH_PROPERTY_KEY = "length";
  static constexpr auto ANGLE_PROPERTY_KEY = "angle";
  static constexpr auto CENTER_PROPERTY_KEY = "center";

protected:
  void on_property_value_changed(Property* property) override;

private:
  std::vector<Point> points() const override;
  bool is_closed() const override;
};

}  // namespace omm
