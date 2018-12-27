#pragma once

#include <vector>
#include <functional>

#include "aspects/propertyowner.h"
#include "managers/manager.h"
#include "scene/scene.h"
#include "managers/propertymanager/userpropertymanager/userpropertydialog.h"

class QTabWidget;

namespace omm
{

class PropertyView;

class PropertyManager : public Manager, public AbstractSelectionObserver
{
  DECLARE_MANAGER_TYPE(PropertyManager)

public:
  explicit PropertyManager(Scene& scene);
  ~PropertyManager();

  void set_selection(const std::set<AbstractPropertyOwner*>& selection) override;
  PropertyView property(const std::string& key);
  void clear();
  void add_user_property();

private:
  QTabWidget* m_tabs;
  std::string m_active_category;
  QAction* m_manage_user_properties_action;
  std::unique_ptr<QMenuBar> make_menu_bar();
  std::set<AbstractPropertyOwner*>  m_current_selection;
};

}  // namespace omm
