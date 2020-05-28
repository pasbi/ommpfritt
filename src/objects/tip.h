#pragma once

#include "objects/abstractpath.h"
#include "properties/propertygroups/markerproperties.h"

namespace omm
{

class Tip : public AbstractPath
{
public:
  explicit Tip(Scene *scene);
  Tip(const Tip& other);
  std::vector<Point> points() const override;
  QString type() const override { return TYPE; }
  bool is_closed() const override;
  void on_property_value_changed(Property* property) override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Tip");

private:
  MarkerProperties m_marker_properties;
};

}  // namespace omm
