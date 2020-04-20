#pragma once

#include <QWidgetAction>

namespace omm
{

class ActionWidget;
class PreferencesTreeValueItem;

class Action : public QAction
{
public:
  explicit Action(const PreferencesTreeValueItem& binding);
};

}  // namespace omm
