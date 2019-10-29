#pragma once

#include <map>
#include <QString>
#include <QObject>
#include <Qt>

namespace omm
{

class Preferences : public QObject
{
  Q_OBJECT
public:
  Preferences();
  ~Preferences();

  struct MouseModifier
  {
    MouseModifier(const QString& label,
                  Qt::MouseButton default_button, Qt::KeyboardModifiers default_modifiers);
    const QString label;
    Qt::MouseButton button;
    Qt::KeyboardModifiers modifiers;
  };

  const std::map<QString, MouseModifier>& mouse_modifiers() const;
  std::map<QString, MouseModifier>& mouse_modifiers();

private:
  std::map<QString, MouseModifier> m_mouse_modifiers;
};

}  // namespace omm
