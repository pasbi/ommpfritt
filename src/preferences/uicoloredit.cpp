#include "preferences/uicoloredit.h"
#include "logging.h"
#include "syncpalettedecorator.h"
#include "ui_uicoloredit.h"
#include "uicolors.h"
#include <QColorDialog>
#include <QPainter>
#include <QStyle>

namespace omm
{
UiColorEdit::UiColorEdit(QWidget* parent) : QWidget(parent), m_ui(new Ui::UiColorEdit)
{
  m_ui->setupUi(this);
  m_ui->pb_reset->setIcon(QIcon(":/icons/revert_128.png"));
  m_ui->pb_reset->setFocusPolicy(Qt::NoFocus);

  connect(m_ui->pb_reset, &QPushButton::clicked, [this]() {
    set_color(m_default_color);
    close();
  });
  SyncPaletteButtonDecorator::decorate(*m_ui->pb_reset);
}

UiColorEdit::~UiColorEdit() = default;

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
  UiColors::draw_background(painter, rect());
  painter.fillRect(rect(), m_color.to_qcolor());
}

void UiColorEdit::mouseDoubleClickEvent(QMouseEvent*)
{
  const QColor color
      = QColorDialog::getColor(m_color.to_qcolor(), this, "", QColorDialog::ShowAlphaChannel);
  if (color.isValid()) {
    set_color(Color(color));
  }
}

}  // namespace omm
