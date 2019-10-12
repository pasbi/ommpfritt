#include "preferences/uicoloredit.h"
#include "ui_uicoloredit.h"
#include <QPainter>
#include <QColorDialog>
#include <QStyle>
#include "logging.h"

namespace omm
{

UiColorEdit::UiColorEdit(QWidget* parent) : QWidget(parent), m_ui(new Ui::UiColorEdit)
{
  m_ui->setupUi(this);
  m_ui->pb_reset->setIcon(QIcon(":/icons/Revert.png"));

  connect(m_ui->pb_reset, &QPushButton::clicked, [this]() {
    set_color(m_default_color);
  });
}


UiColorEdit::~UiColorEdit()
{
}

void UiColorEdit::set_default_color(const Color& color)
{
  m_default_color = color;
}

void UiColorEdit::set_color(const Color& color)
{
  m_color = color;
  update();
}

Color UiColorEdit::color() const
{
  return m_color;
}

void UiColorEdit::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  painter.fillRect(rect(), m_color.to_qcolor());
}

void UiColorEdit::mouseDoubleClickEvent(QMouseEvent*)
{
  const QColor color = QColorDialog::getColor(m_color.to_qcolor(), this, "");
  if (color.isValid()) {
    set_color(Color(color));
  }
}

}  // namespace omm
