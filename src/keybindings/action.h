#pragma once

#include <QWidgetAction>

namespace omm
{

class KeyBinding;

class Action : public QWidgetAction
{
public:
  explicit Action(const KeyBinding& binding);
  QWidget* createWidget(QWidget* parent) override;

private:
  const KeyBinding& m_key_binding;
};

}  // namespace omm