#pragma once

#include <QWidgetAction>

namespace omm
{

class ActionWidget;
class PreferencesTreeValueItem;

class Action : public QWidgetAction
{
public:
  explicit Action(const PreferencesTreeValueItem& binding);
  QWidget* createWidget(QWidget* parent) override;
  void set_highlighted(bool h);

private:
  const PreferencesTreeValueItem& m_key_binding;
  ActionWidget* m_action_widget;
};

}  // namespace omm
