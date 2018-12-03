#include "propertywidgets/colorpropertywidget/coloredit.h"
#include <QColorDialog>
#include <QPaintEvent>
#include <QPainter>

namespace
{

auto to_qcolor(omm::Color color)
{
  color = color.clamped() * 255.0;
  return QColor(color.red(), color.green(), color.blue(), color.alpha());
}

auto from_qcolor(const QColor& color)
{
  const auto vec4 = arma::vec4 {
    color.red() / 255.0, color.green() / 255.0, color.blue() / 255.0, color.alpha() / 255.0
  };
  return omm::Color(vec4);
}

}  // namespace

namespace omm
{

ColorEdit::ColorEdit()
{
}

void ColorEdit::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  painter.fillRect(rect(), to_qcolor(m_current_color));
}

void ColorEdit::set_value(const value_type& value)
{
  if (m_current_color != value) {
    m_current_color = value;
    Q_EMIT color_changed(m_current_color);
    update();
  }
}

void ColorEdit::set_inconsistent_value()
{
  m_is_consistent = true;
  update();
}

ColorEdit::value_type ColorEdit::value() const
{
  return m_current_color;
}

void ColorEdit::mouseDoubleClickEvent(QMouseEvent* event)
{
  const auto color = QColorDialog::getColor(to_qcolor(m_current_color), this);
  if (color.isValid()) {
    set_value(from_qcolor(color));
  }
}

}  // namespace omm
