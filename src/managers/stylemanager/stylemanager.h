#pragma once

#include "managers/itemmanager.h"
#include "managers/stylemanager/stylelistview.h"

namespace omm
{

class StyleManager : public ItemManager<StyleListView>
{
  DECLARE_MANAGER_TYPE(StyleManager)

public:
  explicit StyleManager(Scene& scene);
};

}  // namespace omm
