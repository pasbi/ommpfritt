#include "propertywidgets/colorpropertywidget/coloredit.h"
#include <QColorDialog>
#include <QPaintEvent>
#include <QPainter>

namespace
{

auto to_qcolor(const omm::Color& color)
{
  const auto cast = [](const double t) { return std::clamp(static_cast<int>(t * 255.0), 0, 255); };
  return QColor(cast(color.red()), cast(color.green()), cast(color.blue()), cast(color.alpha()));
}

auto from_qcolor(const QColor& color)
{
  return omm::Color(color.red(), color.green(), color.blue(), color.alpha()) / 255.0;
}

}  // namespace

namespace omm
{

ColorEdit::ColorEdit(const on_value_changed_t& on_value_changed)
  : MultiValueEdit<Color>(on_value_changed) { }

void ColorEdit::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  painter.fillRect(rect(), to_qcolor(m_current_color));
}

void ColorEdit::set_value(const value_type& value)
{
  if (m_current_color != value) {
    m_current_color = value;
    on_value_changed(m_current_color);
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
  const auto color = QColorDialog::getColor(to_qcolor(m_current_color), this);
  if (color.isValid()) {
    set_value(from_qcolor(color));
  }
}

}  // namespace omm
