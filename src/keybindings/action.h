#pragma once

#include <QWidgetAction>

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
  const KeyBinding& m_key_binding;
  ActionWidget* m_action_widget;
};

}  // namespace omm
