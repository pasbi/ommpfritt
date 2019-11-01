#include "preferences/preferences.h"
#include <QSettings>

namespace
{

auto init_mouse_modifiers()
{
  using MouseModifier = omm::Preferences::MouseModifier;
  std::map<QString, MouseModifier> map;
  map.insert({ "zoom viewport", MouseModifier(omm::Preferences::tr("&zoom"),
                                              Qt::RightButton, Qt::AltModifier) });
  map.insert({ "shift viewport", MouseModifier(omm::Preferences::tr("&shift"),
                                               Qt::LeftButton, Qt::AltModifier) });
  return map;
}

auto init_grid_options()
{
  using GridOption = omm::Preferences::GridOption;
  std::map<double, GridOption> map;
  map.insert({ 100.0,   GridOption( omm::Preferences::tr("&fine"),
                                  QColor(0, 0, 0, 100), 0.5)});
  map.insert({ 1000.0,  GridOption( omm::Preferences::tr("&mid"),
                                  QColor(0, 0, 0, 128), 1.0)});
  map.insert({ 10000.0, GridOption( omm::Preferences::tr("&coarse"),
                                  QColor(0, 0, 0, 255), 2.0)});
  return map;
}

template<typename S, typename T>
void set_if_exist(const QSettings& settings, const QString& key, T& value)
{
  if (settings.contains(key)) {
    value = static_cast<T>(settings.value(key).value<S>());
  }
}

}  // namespace

namespace omm
{

Preferences::Preferences()
  : mouse_modifiers(init_mouse_modifiers())
  , grid_options(init_grid_options())
{
  QSettings settings;
  for (const auto& [ key, value ] : mouse_modifiers) {
    const QString prefix = "preferences/" + key;
    set_if_exist<int>(settings, prefix + "/button", mouse_modifiers.at(key).button);
    set_if_exist<int>(settings, prefix + "/modifiers", mouse_modifiers.at(key).modifiers);
  }
  for (const auto& [ key, value ] : grid_options) {
    const QString prefix = "preferences/" + QString("%1").arg(key);
    set_if_exist<QColor>(settings, prefix + "/color", grid_options.at(key).color);
    set_if_exist<double>(settings, prefix + "/pen_width", grid_options.at(key).pen_width);
  }
}

Preferences::~Preferences()
{
  QSettings settings;
  for (const auto& [ key, value ] : mouse_modifiers) {
    const QString prefix = "preferences/" + key;
    settings.setValue(prefix + "/button", static_cast<int>(value.button));
    settings.setValue(prefix + "/modifiers", static_cast<int>(value.modifiers));
  }
  for (const auto& [ key, value ] : grid_options) {
    const QString prefix = "preferences/" + QString("%1").arg(key);
    settings.setValue(prefix + "/color", value.color);
    settings.setValue(prefix + "/pen_width", value.pen_width);
  }
}

Preferences::MouseModifier::MouseModifier(const QString& label, Qt::MouseButton default_button,
                                          Qt::KeyboardModifiers default_modifiers)
  : label(label), button(default_button), modifiers(default_modifiers)
{
}

Preferences::GridOption::GridOption(const QString& label, const QColor& color, double pen_width)
  : label(label), color(color), pen_width(pen_width)
{
}

}  // namespace omm

