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

void StyleManager::on_selection_changed( const QItemSelection& selection,
                                         const QItemSelection& old_selection )
{
  for (auto& index : old_selection.indexes()) {
    if (!selection.contains(index)) {
      m_scene.style(index.row()).deselect();
    }
  }
  for (auto& index : selection.indexes()) {
    m_scene.style(index.row()).select();
  }
}

}  // namespace omm
