#pragma once

#include <vector>
#include <functional>

#include "aspects/propertyowner.h"
#include "managers/manager.h"
#include "scene/scene.h"

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
  PropertyView property(const AbstractPropertyOwner::Key& key);
  void clear();

private:
  QTabWidget& m_tabs;
};

}  // namespace omm
