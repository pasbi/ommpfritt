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

class PropertyManager
  : public Manager, public AbstractPropertyObserver
{
public:
  explicit PropertyManager(Scene& scene);
  ~PropertyManager();

  void on_selection_changed(const std::set<AbstractPropertyOwner*>& selection) override;
  PropertyView property(const std::string& key);
  void clear();
  void add_user_property();
  void on_property_value_changed(Property& property) override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "PropertyManager");
  std::string type() const override;

private:
  QTabWidget* m_tabs;
  std::string m_active_category;
  QAction* m_manage_user_properties_action;
  std::unique_ptr<QMenuBar> make_menu_bar();
  std::set<AbstractPropertyOwner*>  m_current_selection;
  std::set<Property*> m_observed_properties;
  std::string make_window_title() const;
};

}  // namespace omm
