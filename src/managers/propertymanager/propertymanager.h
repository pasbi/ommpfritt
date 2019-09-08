#pragma once

#include <vector>
#include <functional>
#include <QScrollArea>

#include "widgets/multitabbar.h"
#include "aspects/propertyowner.h"
#include "managers/manager.h"
#include "scene/scene.h"
#include "managers/propertymanager/userpropertymanager/userpropertydialog.h"

class QTabWidget;

namespace omm
{

class PropertyView;
class PropertyManagerTab;

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
  OrderedMap<std::string, PropertyManagerTab> m_tabs;
  QAction* m_manage_user_properties_action;
  std::unique_ptr<QWidget> make_menu_bar();
  std::set<AbstractPropertyOwner*> m_current_selection;
  std::string make_window_title() const;
  bool m_is_locked = false;
  QVBoxLayout* m_layout;
  std::unique_ptr<QScrollArea> m_scroll_area;
  std::unique_ptr<MultiTabBar> m_tab_bar;

  std::map<std::set<AbstractPropertyOwner*>, std::set<int>> m_current_categroy_indices;

private Q_SLOTS:
  void set_selection(const std::set<AbstractPropertyOwner*>& selection);
  void set_locked(bool locked);
  void activate_tabs(const std::set<int>& indices);
public Q_SLOTS:
  void update_property_widgets();
};

}  // namespace omm
