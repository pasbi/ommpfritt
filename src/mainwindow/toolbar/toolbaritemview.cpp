#include "mainwindow/toolbar/toolbaritemview.h"
#include "logging.h"
#include "mainwindow/toolbar/toolbaritemmodel.h"
#include <QDropEvent>

namespace omm
{
void ToolBarItemView::setModel(QAbstractItemModel* model)
{
  assert(model->inherits(ToolBarItemModel::staticMetaObject.className()));
  QTreeView::setModel(model);
}

void ToolBarItemView::dropEvent(QDropEvent* event)
{
  // the default implementation does remove the dragged items in a wrong way.

  const bool remove_selection = event->source() == this;
  auto* model = dynamic_cast<ToolBarItemModel*>(this->model());

  std::set<const QStandardItem*> items_to_remove;
  if (remove_selection) {
    const auto indices = selectionModel()->selection().indexes();
    for (const QModelIndex& index : indices) {
      items_to_remove.insert(model->itemFromIndex(index));
    }
  }

  QTreeView::dropEvent(event);

  if (remove_selection) {
    QItemSelection selection;
    for (const QStandardItem* item : items_to_remove) {
      const QModelIndex index = model->indexFromItem(item);
      selection.merge(QItemSelection(index, index), QItemSelectionModel::Select);
    }

    model->remove_selection(selection);
  }
}

}  // namespace omm
