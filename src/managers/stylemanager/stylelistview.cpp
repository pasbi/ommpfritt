#include "managers/stylemanager/stylelistview.h"
#include "scene/stylelist.h"
#include "scene/scene.h"

namespace omm
{

void StyleListView::mouseReleaseEvent(QMouseEvent *e)
{
  this->model()->scene.set_selection(this->selected_items());
  ManagerItemView::mouseReleaseEvent(e);
}

void StyleListView::set_selection(const std::set<Style*>& selection)
{
  QItemSelection new_selection;
  for (Style* style : selection) {
    QModelIndex index = model()->index_of(*style);
    new_selection.merge(QItemSelection(index, index), QItemSelectionModel::Select);
  }
  selectionModel()->select(new_selection, QItemSelectionModel::ClearAndSelect);
}

}  // namespace omm
