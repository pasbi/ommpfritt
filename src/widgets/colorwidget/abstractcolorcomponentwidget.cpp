#include "widgets/colorwidget/abstractcolorcomponentwidget.h"

#include <QMouseEvent>
#include <QPainter>

namespace omm
{

void AbstractColorComponentWidget::set_role(Color::Role role)
{
  m_role = role;
}

void AbstractColorComponentWidget::set_color(const Color& color)
{
  if ((m_color - color).abs().max_component() > 0.00001) {
    m_color = color;
    Q_EMIT color_changed(color);
  }
}

}  // namespace omm
