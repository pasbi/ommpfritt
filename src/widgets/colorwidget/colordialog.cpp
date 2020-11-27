#include "widgets/colorwidget/colordialog.h"
#include "logging.h"
#include "widgets/colorwidget/colorwidget.h"
#include <QDialogButtonBox>
#include <QResizeEvent>
#include <QVBoxLayout>

namespace omm
{
ColorDialog::ColorDialog(QWidget* parent) : QDialog(parent)
{
  auto layout = std::make_unique<QVBoxLayout>();

  auto color_widget = std::make_unique<ColorWidget>();
  m_color_widget = color_widget.get();
  layout->addWidget(color_widget.release());

  auto button_box
      = std::make_unique<QDialogButtonBox>(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(button_box.get(), &QDialogButtonBox::accepted, this, &ColorDialog::accept);
  connect(button_box.get(), &QDialogButtonBox::rejected, this, &ColorDialog::reject);
  layout->addWidget(button_box.release());

  setLayout(layout.release());
  static constexpr int DEFAULT_WIDTH = 841;
  static constexpr int DEFAULT_HEIGHT = 439;
  resize(QSize(DEFAULT_WIDTH, DEFAULT_HEIGHT));
}

ColorDialog::~ColorDialog() = default;

std::pair<Color, bool> ColorDialog::get_color(const Color& old, QWidget* parent)
{
  ColorDialog dialog(parent);
  dialog.set_color(old);
  switch (dialog.exec()) {
  case QDialog::Accepted:
    return {dialog.color(), true};
  case QDialog::Rejected:
    return {Color(), false};
  default:
    Q_UNREACHABLE();
    return {Color(), false};
  }
}

void ColorDialog::set_color(const Color& color)
{
  m_color_widget->set_color(color);
}

Color ColorDialog::color() const
{
  return m_color_widget->color();
}

}  // namespace omm
