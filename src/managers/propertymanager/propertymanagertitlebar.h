#pragma once

#include <set>
#include <QWidget>
#include "managers/managertitlebar.h"

class QPushButton;

namespace omm
{

class PropertyManager;
class AbstractPropertyOwner;

class PropertyManagerTitleBar : public ManagerTitleBar
{
  Q_OBJECT
public:
  explicit PropertyManagerTitleBar(PropertyManager& parent);
  void set_selection(const std::set<AbstractPropertyOwner*>& selection);

private:
  AbstractPropertyOwner* m_first_selected = nullptr;
  QAction* m_manage_user_properties_action;
  QPushButton* m_lock_button;

private Q_SLOTS:
  void update_lock_button_icon(bool checked);
};

}  // namespace omm
