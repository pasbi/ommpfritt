#include "managers/stylemanager/stylemanager.h"

#include <QEvent>
#include "renderers/style.h"
#include "managers/stylemanager/stylelistview.h"

namespace omm
{

StyleManager::StyleManager(Scene& scene)
  : ItemManager(tr("Styles"), scene)
{
  setWindowTitle(tr("style manager"));
  setObjectName(TYPE());
}

}  // namespace omm
