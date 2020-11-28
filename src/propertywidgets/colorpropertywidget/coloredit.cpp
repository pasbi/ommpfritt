#include "propertywidgets/colorpropertywidget/coloredit.h"
#include "logging.h"
#include "preferences/uicolors.h"
#include "widgets/colorwidget/colordialog.h"
#include <QColorDialog>
#include <QPaintEvent>
#include <QPainter>

namespace omm
{
void ColorEdit::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  UiColors::draw_background(painter, rect());
  painter.fillRect(rect(), m_current_color.to_qcolor());
  {
    static const QMargins text_margins(3, 3, 3, 3);
    const auto rect = this->rect().marginsRemoved(text_margins);
    const auto elided_text = painter.fontMetrics().elidedText(text, Qt::ElideMiddle, rect.width());
    painter.setPen(m_contrast_color);
    if (!isEnabled()) {
      QFont font = painter.font();
      font.setStrikeOut(true);
      painter.setFont(font);
    }
    painter.drawText(rect, Qt::AlignCenter, elided_text);
  }
}

ColorEdit::ColorEdit() : m_contrast_color(compute_contrast_color())
{
}

void ColorEdit::set_value(const value_type& value)
{
  if (m_current_color != value) {
    m_current_color = value;
    Q_EMIT value_changed(value);
    m_contrast_color = compute_contrast_color();
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

void ColorEdit::mouseDoubleClickEvent(QMouseEvent*)
{
  const auto [color, accepted] = ColorDialog::get_color(m_current_color, nullptr);
  if (accepted) {
    set_value(color);
  }
}

QColor ColorEdit::compute_contrast_color() const
{
  static constexpr double LIGHTNESS_THRESHOLD = 0.5;
  auto color = ColorEdit::value();
  if (color.components(Color::Model::HSVA)[2] > LIGHTNESS_THRESHOLD) {
    return Qt::black;
  } else {
    return Qt::white;
  }
}

}  // namespace omm
