#pragma once

#include "properties/propertygroups/propertygroup.h"

namespace omm
{
class TextOptionProperties : public PropertyGroup
{
public:
  using PropertyGroup::PropertyGroup;
  void make_properties(const QString& category) const override;
  QTextOption get_option() const;

  static constexpr auto ALIGNH_PROPERTY_KEY = "halign";
  static constexpr auto ALIGNV_PROPERTY_KEY = "valign";
  static constexpr auto DIRECTION_PROPERTY_KEY = "direction";
  static constexpr auto TABSTOPDISTANCE_PROPERTY_KEY = "tabstopdistance";
  static constexpr auto WRAP_MODE_PROPERTY_KEY = "wrap";
};

}  // namespace omm
