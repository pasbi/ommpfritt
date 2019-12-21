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
  QPushButton* m_open_user_properties_dialog_button;
};

}  // namespace omm
