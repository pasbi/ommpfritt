#include "preferences/uicoloredit.h"
#include "ui_uicoloredit.h"
#include <QPainter>
#include <QColorDialog>

namespace omm
{

UiColorEdit::UiColorEdit(QWidget* parent) : QWidget(parent), m_ui(new Ui::UiColorEdit)
{
}


UiColorEdit::~UiColorEdit()
{
}

void UiColorEdit::set_color(const Color& color)
{
  m_color = color;
}

Color UiColorEdit::color() const
{
  return m_color;
}

void UiColorEdit::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  painter.fillRect(rect(), m_color);
}

void UiColorEdit::mouseDoubleClickEvent(QMouseEvent*)
{
  const QColor color = QColorDialog::getColor(m_color, this, "");
  if (color.isValid()) {
    set_color(color);
  }
}

}  // namespace omm
