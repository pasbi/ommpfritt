#include "managers/stylemanager/stylemanager.h"

#include <QEvent>
#include "renderers/style.h"
#include "managers/stylemanager/stylelistview.h"
#include "scene/listadapter.h"
#include "scene/scene.h"

namespace omm
{

StyleManager::StyleManager(Scene& scene)
  : ItemManager(tr("Styles"), scene, scene.style_list_adapter)
{
  setWindowTitle(tr("style manager"));
  setObjectName(TYPE());
}

}  // namespace omm
