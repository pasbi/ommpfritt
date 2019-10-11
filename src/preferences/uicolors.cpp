#include "preferences/uicolors.h"
#include <QColor>

namespace omm
{

UiColors::UiColors() : PreferencesTree(":/ui-colors-dark.cfg")
{

}

UiColors::~UiColors()
{

}

QVariant UiColors::data(const PreferencesTreeValueItem& value, int role) const
{
  switch (role) {
  case Qt::DisplayRole:
    return QColor(Qt::red);
  case Qt::EditRole:
    return QString::fromStdString(value.value());
  default:
    return QVariant();
  }
}

}  // namespace omm
