#pragma once

#include <QLabel>
#include <QScrollArea>
#include <functional>
#include <vector>

#include "managers/manager.h"
#include "managers/propertymanager/propertymanagertab.h"
#include "managers/propertymanager/userpropertydialog.h"
#include "orderedmap.h"
#include "widgets/multitabbar.h"

class QTabWidget;

namespace omm
{
class AbstractPropertyOwner;
class PropertyView;
class PropertyManagerTab;
class PropertyManagerTitleBar;

class PropertyManager : public Manager
{
  Q_OBJECT
public:
  explicit PropertyManager(Scene& scene);
  PropertyManager(PropertyManager&&) = delete;
  PropertyManager(const PropertyManager&) = delete;
  PropertyManager& operator=(PropertyManager&&) = delete;
  PropertyManager& operator=(const PropertyManager&) = delete;
  ~PropertyManager() override;

  PropertyView property(const QString& key);
  void clear();
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "PropertyManager");
  [[nodiscard]] QString type() const override;
  bool perform_action(const QString& name) override;

private:
  OrderedMap<QString, PropertyManagerTab> m_tabs;
  std::set<AbstractPropertyOwner*> m_current_selection;
  [[nodiscard]] QString make_window_title() const;
  QVBoxLayout* m_layout;
  std::unique_ptr<QScrollArea> m_scroll_area;
  std::unique_ptr<MultiTabBar> m_tab_bar;
  std::unique_ptr<QLabel> m_icon_label;
  std::unique_ptr<QLabel> m_selection_label;

  std::map<std::set<AbstractPropertyOwner*>, std::set<int>> m_current_categroy_indices;
  PropertyManagerTitleBar* m_title_bar;

private:
  void activate_tabs(const std::set<int>& indices);

public:
  void set_selection(const std::set<AbstractPropertyOwner*>& selection);
  void update_property_widgets();
};

}  // namespace omm
