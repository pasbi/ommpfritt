#include "preferences/uicolors.h"
#include <QColor>
#include "color/color.h"
#include <QApplication>
#include <QPalette>
#include "logging.h"

namespace omm
{

UiColors::UiColors() : PreferencesTree(":/ui-colors-dark.cfg")
{
}

UiColors::~UiColors()
{
}

std::string UiColors::decode_data(const QVariant& value) const
{
  return Color(value.value<QColor>()).to_hex();
}

QVariant UiColors::encode_data(const PreferencesTreeValueItem& item, int role) const
{
  switch (role) {
  case Qt::DisplayRole:
    return QVariant();
  case Qt::BackgroundRole:
    return Color(item.value()).to_qcolor();
  case Qt::EditRole:
    return QString::fromStdString(Color(item.value()).to_hex());
  default:
    return QVariant();
  }
}

QPalette UiColors::make_palette() const
{
  const auto get_widget_color = [this](const std::string& name) {
    return Color(value("widget", name)->value()).to_qcolor();
  };
  QPalette palette = qApp->palette();
  palette.setBrush(QPalette::Window, get_widget_color("background"));
  return palette;
}

Color UiColors::color(const QModelIndex& index) const
{
  assert(!is_group(index));
  return Color(data(index, Qt::EditRole).toString().toStdString());
}

void UiColors::set_color(const QModelIndex& index, const Color& color)
{
  const bool s = setData(index, color.to_qcolor(), Qt::EditRole);
  assert(s);
  Q_UNUSED(s)
}

}  // namespace omm
