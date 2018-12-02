#pragma once

#include "aspects/propertyowner.h"
#include "color/color.h"

namespace omm
{

class Style : public PropertyOwner<AbstractPropertyOwner::Kind::Style>
{
public:
  explicit Style();
  std::string name() const override;

  static constexpr auto PEN_IS_ACTIVE_KEY = "pen/active";
  static constexpr auto PEN_COLOR_KEY = "pen/color";
  static constexpr auto PEN_WIDTH_KEY = "pen/width";
  static constexpr auto BRUSH_IS_ACTIVE_KEY = "brush/active";
  static constexpr auto BRUSH_COLOR_KEY = "brush/color";
};

}  // namespace omm
