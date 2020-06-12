#pragma once

#include "tags/tag.h"
#include <Qt>

namespace omm
{

class PathTag : public Tag
{
public:
  explicit PathTag(Object& owner);
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "PathTag");
  static constexpr auto PATH_REFERENCE_PROPERTY_KEY = "path";
  static constexpr auto ALIGN_REFERENCE_PROPERTY_KEY = "align";
  static constexpr auto SEGMENT_PROPERTY_KEY = "segment";
  static constexpr auto POSITION_PROPERTY_KEY = "t";
  static constexpr auto INTERPOLATION_KEY = "interpolation";
  void evaluate() override;
  Flag flags() const override;
  void on_property_value_changed(Property* property) override;

};

}  // namespace omm
