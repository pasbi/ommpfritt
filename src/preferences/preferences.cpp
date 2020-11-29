#include "preferences/preferences.h"
#include <QMouseEvent>
#include <QSettings>

namespace
{
auto init_mouse_modifiers()
{
  using MouseModifier = omm::Preferences::MouseModifier;
  std::map<QString, MouseModifier> map;
  map.insert({"zoom viewport",
              MouseModifier(omm::Preferences::tr("&zoom"), Qt::RightButton, Qt::AltModifier)});
  map.insert({"shift viewport",
              MouseModifier(omm::Preferences::tr("&shift"), Qt::LeftButton, Qt::AltModifier)});
  return map;
}

auto init_grid_options()
{
  using GridOption = omm::Preferences::GridOption;
  std::map<QString, GridOption> map;
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
  map.insert({"fine", GridOption(omm::Preferences::tr("&fine"), Qt::SolidLine, 0.5, 100.0)});
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
  map.insert({"mid", GridOption(omm::Preferences::tr("&mid"), Qt::SolidLine, 1.0, 1000.0)});
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
  map.insert({"coarse", GridOption(omm::Preferences::tr("&coarse"), Qt::SolidLine, 2.0, 10000.0)});
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
    : mouse_modifiers(init_mouse_modifiers()), grid_options(init_grid_options())
{
  QSettings settings;
  for (const auto& [key, value] : mouse_modifiers) {
    const QString prefix = "preferences/" + key;
    set_if_exist<int>(settings, prefix + "/button", mouse_modifiers.at(key).button);
    set_if_exist<int>(settings, prefix + "/modifiers", mouse_modifiers.at(key).modifiers);
  }
  for (const auto& [key, value] : grid_options) {
    const QString prefix = "preferences/" + QString("%1").arg(key);
    set_if_exist<int>(settings, prefix + "/pen_style", grid_options.at(key).pen_style);
    set_if_exist<double>(settings, prefix + "/pen_width", grid_options.at(key).pen_width);
    set_if_exist<int>(settings, prefix + "/zorder", grid_options.at(key).zorder);
  }
}

Preferences::~Preferences()
{
  QSettings settings;
  for (const auto& [key, value] : mouse_modifiers) {
    const QString prefix = "preferences/" + key;
    settings.setValue(prefix + "/button", static_cast<int>(value.button));
    settings.setValue(prefix + "/modifiers", static_cast<int>(value.modifiers));
  }
  for (const auto& [key, value] : grid_options) {
    const QString prefix = "preferences/" + QString("%1").arg(key);
    settings.setValue(prefix + "/pen_style", static_cast<int>(value.pen_style));
    settings.setValue(prefix + "/pen_width", value.pen_width);
    settings.setValue(prefix + "/zorder", static_cast<int>(value.zorder));
  }
}

bool Preferences::match(const QString& key, const QMouseEvent& event, bool check_modifiers) const
{
  const auto mm = mouse_modifiers.at(key);
  if (check_modifiers) {
    return mm.button == event.button() && mm.modifiers == event.modifiers();
  } else {
    return mm.modifiers == event.modifiers();
  }
}

Preferences::MouseModifier::MouseModifier(const QString& label,
                                          Qt::MouseButton default_button,
                                          Qt::KeyboardModifiers default_modifiers)
    : label(label), button(default_button), modifiers(default_modifiers)
{
}

Preferences::GridOption::GridOption(const QString& label,
                                    const Qt::PenStyle& pen_style,
                                    double pen_width,
                                    double base)
    : label(label), pen_style(pen_style), pen_width(pen_width), base(base)
{
}

}  // namespace omm
