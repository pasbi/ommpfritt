#include "renderers/style.h"

namespace omm
{

Style::Style()
  : is_pen_active(false)
  , pen_color(Color::BLACK)
  , pen_width(1.0)
  , is_brush_active(false)
  , brush_color(Color::BLUE)
{
  // add_property(color, "pen", "color")
  // add_property(double, "pen", "width")
  // add_property(color, "brush", "width")

}

std::string Style::name() const
{
  return "Style";
}

}  // namespace omm
