#include "propertywidgets/colorpropertywidget/coloredit.h"
#include <QColorDialog>
#include <QPaintEvent>
#include <QPainter>

namespace omm
{

void ColorEdit::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  painter.fillRect(rect(), m_current_color);
}

void ColorEdit::set_value(const value_type& value)
{
  if (m_current_color != value) {
    m_current_color = value;
    Q_EMIT value_changed(value);
    update();
  }
}

void ColorEdit::set_inconsistent_value()
{
  m_is_consistent = true;
  update();
}

ColorEdit::value_type ColorEdit::value() const { return m_current_color; }

void ColorEdit::mouseDoubleClickEvent(QMouseEvent*)
{
  const auto color = QColorDialog::getColor(m_current_color, this);
  if (color.isValid()) {
    set_value(color);
  }
}

}  // namespace omm
