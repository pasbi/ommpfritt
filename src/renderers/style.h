#pragma once

#include "properties/hasproperties.h"
#include "color/color.h"

namespace omm
{

class Style : public HasProperties
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
