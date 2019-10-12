#include "preferences/uicolors.h"
#include <iostream>
#include <QColor>
#include "color/color.h"
#include <QApplication>
#include <QPalette>
#include "logging.h"

namespace omm
{

UiColors::UiColors() : PreferencesTree(":/skins/ui-colors-dark.cfg")
{
  load_from_qsettings("ui-colors");

}

UiColors::~UiColors()
{
  save_in_qsettings("ui-colors");
}

QVariant UiColors::data(int column, const PreferencesTreeValueItem& item, int role) const
{
  switch (role) {
  case Qt::DisplayRole:
    return QVariant();
  case Qt::BackgroundRole:
    return Color(item.value(column-1)).to_qcolor();
  case Qt::EditRole:
    return QString::fromStdString(Color(item.value(column-1)).to_hex());
  case DEFAULT_VALUE_ROLE:
    return QString::fromStdString(Color(item.default_value(column-1)).to_hex());
  default:
    return QVariant();
  }
}

bool UiColors::set_data(int column, PreferencesTreeValueItem& item, const QVariant& value)
{
  item.set_value(Color(value.value<QColor>()).to_hex(), column-1);
  return true;
}

QPalette UiColors::make_palette() const
{
  static const std::map<std::string, QPalette::ColorRole> role_map {
    { "window",           QPalette::Window },
    { "window text",      QPalette::WindowText },
    { "text",             QPalette::Text},
    { "base",             QPalette::Base },
    { "alternate base",   QPalette::AlternateBase },
    { "placeholder text", QPalette::PlaceholderText },
    { "button",           QPalette::Button },
    { "button text",      QPalette::ButtonText },
    { "light",            QPalette::Light },
    { "midlight",         QPalette::Midlight },
    { "dark",             QPalette::Dark },
    { "mid",              QPalette::Mid },
    { "shadow",           QPalette::Shadow },
    { "bright text",      QPalette::BrightText },
    { "highlight",        QPalette::Highlight },
    { "highlighted text", QPalette::HighlightedText },
    { "link",             QPalette::Link },
    { "link visited",     QPalette::LinkVisited },
  };

  static const std::map<std::size_t, QPalette::ColorGroup> group_map {
      { 0, QPalette::Active },
      { 1, QPalette::Inactive },
      { 2, QPalette::Disabled }
  };

  const auto get_widget_color = [this](std::size_t column, const std::string& name) {
    return Color(value("Widget", name)->value(column)).to_qcolor();
  };
  QPalette palette = qApp->palette();
  for (auto&& [ name, role ] : role_map) {
    for (auto&& [column, group] : group_map) {
      palette.setBrush(group, role, get_widget_color(column, name));
    }
  }
  return palette;
}

int UiColors::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return 4;
}

QVariant UiColors::headerData(int section, Qt::Orientation orientation, int role) const
{
  Q_UNUSED(orientation)
  switch (role) {
  case Qt::DisplayRole:
    switch (section) {
    case 0:
      return tr("");
    case 1:
      return tr("Active");
    case 2:
      return tr("Inactive");
    case 3:
      return tr("Disabled");
    }
  default:
    return QVariant();
  }
}

}  // namespace omm
