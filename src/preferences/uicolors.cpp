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

QVariant UiColors::data(const PreferencesTreeValueItem& value, int column, int role) const
{
  switch (role) {
  case Qt::DisplayRole:
    [[fallthrough]];
  case Qt::BackgroundRole:
    if (column == 0) {
      return QVariant();
    } else {
      [[fallthrough]];
    }
  case Qt::EditRole:
    return QColor(Color(value.value()));
  default:
    return QVariant();
  }
}

std::string UiColors::set_data(const QVariant& value) const
{
  return Color(value.value<QColor>()).to_hex();
}

}  // namespace omm
