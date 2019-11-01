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
  : m_mouse_modifiers(init_mouse_modifiers())
{
  QSettings settings;
  for (const auto& [ key, value ] : m_mouse_modifiers) {
    const QString prefix = "preferences/" + key;
    set_if_exist<int>(settings, prefix + "/button", m_mouse_modifiers.at(key).button);
    set_if_exist<int>(settings, prefix + "/modifiers", m_mouse_modifiers.at(key).modifiers);
  }
}

Preferences::~Preferences()
{
  QSettings settings;
  for (const auto& [ key, value ] : m_mouse_modifiers) {
    const QString prefix = "preferences/" + key;
    settings.setValue(prefix + "/button", static_cast<int>(value.button));
    settings.setValue(prefix + "/modifiers", static_cast<int>(value.modifiers));
  }
}

const std::map<QString, Preferences::MouseModifier>& Preferences::mouse_modifiers() const
{
  return m_mouse_modifiers;
}

std::map<QString, Preferences::MouseModifier>& Preferences::mouse_modifiers()
{
  return m_mouse_modifiers;
}

Preferences::MouseModifier::MouseModifier(const QString& label, Qt::MouseButton default_button,
                                          Qt::KeyboardModifiers default_modifiers)
  : label(label), button(default_button), modifiers(default_modifiers)
{
}

}  // namespace omm

