#include "managers/stylemanager/stylelistview.h"

namespace omm
{

void StyleListView::set_selection(const std::set<Style*>& selection)
{
  QItemSelection new_selection;
  for (Style* style : selection) {
    QModelIndex index = model()->index_of(*style);
    new_selection.merge(QItemSelection(index, index), QItemSelectionModel::Select);
  }
  selectionModel()->select(new_selection, QItemSelectionModel::ClearAndSelect);
}

}
