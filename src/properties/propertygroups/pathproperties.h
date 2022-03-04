#pragma once

#include "properties/propertygroups/propertygroup.h"
#include "geometry/objecttransformation.h"

namespace omm
{
class PathProperties : public PropertyGroup
{
public:
  using PropertyGroup::PropertyGroup;
  void make_properties(const QString& category) const override;

  static constexpr auto PATH_REFERENCE_PROPERTY_KEY = "path";
  static constexpr auto ALIGN_PROPERTY_KEY = "align";
  static constexpr auto SEGMENT_PROPERTY_KEY = "segment";
  static constexpr auto INTERPOLATION_KEY = "interpolation";
  static const std::set<QString> keys;

  void apply_transformation(Object& o, double t, const ObjectTransformation& transformation) const;
};

}  // namespace omm
