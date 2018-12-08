#pragma once

#include "managers/itemmanager.h"
#include "managers/stylemanager/stylelistadapter.h"

namespace omm
{

class StyleListView;

class StyleManager : public ItemManager<StyleListView, StyleListAdapter>
{
  DECLARE_MANAGER_TYPE(StyleManager)

public:
  explicit StyleManager(Scene& scene);
};

}  // namespace omm
