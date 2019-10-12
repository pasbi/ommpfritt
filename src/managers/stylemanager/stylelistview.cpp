#include "managers/stylemanager/stylelistview.h"
#include "scene/stylelist.h"
#include "scene/scene.h"

#include <QResizeEvent>

namespace omm
{

StyleListView::StyleListView(StyleList& model)
  : ManagerItemView(model)
{
  setWrapping(true);
}

void StyleListView::mouseReleaseEvent(QMouseEvent *e)
{
  this->model()->scene.set_selection(this->selected_items());
  ManagerItemView::mouseReleaseEvent(e);
}

void StyleListView::resizeEvent(QResizeEvent* e)
{
  if (e->size().width() > e->size().height()) {
     setFlow(QListView::LeftToRight);
  } else {
    setFlow(QListView::TopToBottom);
  }
  ManagerItemView::resizeEvent(e);
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
