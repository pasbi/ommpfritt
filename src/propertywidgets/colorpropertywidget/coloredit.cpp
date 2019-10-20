#include "propertywidgets/colorpropertywidget/coloredit.h"
#include "logging.h"
#include <QColorDialog>
#include <QPaintEvent>
#include <QPainter>
#include "widgets/colorwidget/colordialog.h"
#include "preferences/uicolors.h"

namespace omm
{

void ColorEdit::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  UiColors::draw_background(painter, rect());
  painter.fillRect(rect(), m_current_color.to_qcolor());
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
  const auto [color, accepted] = ColorDialog::get_color(m_current_color, this);
  if (accepted) {
    set_value(color);
  }
}

}  // namespace omm
