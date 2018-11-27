#pragma once

#include "aspects/propertyowner.h"
#include "color/color.h"

namespace omm
{

class Style : public PropertyOwner<AbstractPropertyOwner::Kind::Style>
{
public:
  explicit Style();
  bool is_pen_active;
  Color pen_color;
  double pen_width;

  bool is_brush_active;
  Color brush_color;

  std::string name() const override;
};

}  // namespace omm
