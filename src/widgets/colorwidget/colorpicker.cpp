#include "widgets/colorwidget/colorpicker.h"

namespace omm
{
Color ColorPicker::color() const
{
  return m_color;
}

void ColorPicker::set_color(const Color& color)
{
  if (color != m_color) {
    m_color = color;
    Q_EMIT color_changed(color);
  }
}

}  // namespace omm
