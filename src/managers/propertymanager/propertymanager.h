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

class PropertyManager : public Manager
{
  Q_OBJECT
public:
  explicit PropertyManager(Scene& scene);
  ~PropertyManager();

  PropertyView property(const std::string& key);
  void clear();
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "PropertyManager");
  std::string type() const override;

private:
  QTabWidget* m_tabs;
  std::string m_active_category;
  QAction* m_manage_user_properties_action;
  std::unique_ptr<QWidget> make_menu_bar();
  std::set<AbstractPropertyOwner*> m_current_selection;
  std::string make_window_title() const;
  bool m_is_locked = false;

private Q_SLOTS:
  void set_selection(const std::set<AbstractPropertyOwner*>& selection);
  void set_locked(bool locked);

};

}  // namespace omm
