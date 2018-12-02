#include "managers/stylemanager/stylemanager.h"
#include "managers/stylemanager/stylelistview.h"
#include "renderers/style.h"

namespace omm
{

StyleManager::StyleManager(Scene& scene)
  : Manager(tr("Styles"), scene)
  , m_style_list_adapter(scene)
{
  setWindowTitle(tr("style manager"));

  auto list_view = std::make_unique<StyleListView>(scene);
  list_view->set_model(&m_style_list_adapter);

  connect( list_view->selectionModel(), &QItemSelectionModel::selectionChanged,
           this, &StyleManager::on_selection_changed );

  connect( list_view.get(), &StyleListView::mouse_released, [this]() {
    m_scene.selection_changed();
  });

  setWidget(list_view.release());
  setObjectName(TYPE());
}

StyleManager::~StyleManager()
{
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
