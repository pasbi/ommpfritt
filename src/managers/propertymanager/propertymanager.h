#pragma once

#include <vector>
#include <functional>
#include <QScrollArea>
#include <QLabel>

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
class PropertyManagerTitleBar;

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
  bool perform_action(const std::string &name) override;

private:
  OrderedMap<std::string, PropertyManagerTab> m_tabs;
  std::set<AbstractPropertyOwner*> m_current_selection;
  std::string make_window_title() const;
  bool m_is_locked = false;
  QVBoxLayout* m_layout;
  std::unique_ptr<QScrollArea> m_scroll_area;
  std::unique_ptr<MultiTabBar> m_tab_bar;
  std::unique_ptr<QLabel> m_icon_label;
  std::unique_ptr<QLabel> m_selection_label;

  std::map<std::set<AbstractPropertyOwner*>, std::set<int>> m_current_categroy_indices;
  PropertyManagerTitleBar* m_title_bar;

private Q_SLOTS:
  void set_selection(const std::set<AbstractPropertyOwner*>& selection);
  void activate_tabs(const std::set<int>& indices);

public Q_SLOTS:
  void set_locked(bool locked);
  void update_property_widgets();
};

}  // namespace omm
