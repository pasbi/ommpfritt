#pragma once

#include <QWidgetAction>
#include "keybindings/keybinding.h"

namespace omm
{

class ActionWidget;
class KeyBinding;

class Action : public QWidgetAction
{
public:
  explicit Action(const KeyBinding& binding);
  QWidget* createWidget(QWidget* parent) override;
  void set_highlighted(bool h);

private:
  KeyBinding m_key_binding;
  ActionWidget* m_action_widget;
};

}  // namespace omm
