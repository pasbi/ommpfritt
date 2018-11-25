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
  Q_OBJECT
  DECLARE_MANAGER_TYPE(PropertyManager)

public:
  explicit PropertyManager(Scene& scene);
  ~PropertyManager();

  void set_selection(const std::set<PropertyOwner*>& selection) override;
  PropertyView property(const PropertyOwner::Key& key);
  void clear();

private:
  QTabWidget& m_tabs;
};

}  // namespace omm
