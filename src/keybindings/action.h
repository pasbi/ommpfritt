#pragma once

#include <QWidgetAction>

namespace omm
{

class ActionWidget;
class SettingsTreeValueItem;

class Action : public QWidgetAction
{
public:
  explicit Action(const SettingsTreeValueItem& binding);
  QWidget* createWidget(QWidget* parent) override;
  void set_highlighted(bool h);

private:
  const SettingsTreeValueItem& m_key_binding;
  ActionWidget* m_action_widget;
};

}  // namespace omm
