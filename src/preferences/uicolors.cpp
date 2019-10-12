#include "preferences/uicolors.h"
#include <QColor>
#include "color/color.h"

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

std::string UiColors::set_data(const QVariant& value) const
{
  return Color(value.value<QColor>()).to_hex();
}

}  // namespace omm
