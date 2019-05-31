#include "managers/manageritemview.h"

#include <QApplication>
#include <QMessageBox>
#include <QContextMenuEvent>
#include <QMenu>
#include "renderers/style.h"
#include "commands/propertycommand.h"
#include "commands/removecommand.h"
#include "properties/referenceproperty.h"

#include "managers/stylemanager/stylelistview.h"
#include "scene/listadapter.h"
#include "managers/objectmanager/objecttreeview.h"
#include "scene/objecttreeadapter.h"
#include "scene/scene.h"
#include "mainwindow/application.h"

namespace omm
{

template<typename ItemViewT, typename ItemModelT>
ManagerItemView<ItemViewT, ItemModelT>::ManagerItemView(ItemModelT& model)
{
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);
  this->setDragEnabled(true);
  this->setDefaultDropAction(Qt::MoveAction);
  this->viewport()->setAcceptDrops(true);

  ItemViewT::setModel(&model);
}

template<typename ItemViewT, typename ItemModelT>
ItemModelT* ManagerItemView<ItemViewT, ItemModelT>::model() const
{
  return static_cast<ItemModelT*>(ItemViewT::model());
}

template<typename ItemViewT, typename ItemModelT>
void ManagerItemView<ItemViewT, ItemModelT>::mouseReleaseEvent(QMouseEvent* e)
{
  ItemViewT::mouseReleaseEvent(e);
  this->model()->scene.set_selection(this->selected_items());
}

template<typename ItemViewT, typename ItemModelT>
void ManagerItemView<ItemViewT, ItemModelT>::mousePressEvent(QMouseEvent *e)
{
  ItemViewT::mousePressEvent(e);
  if (e->button() == Qt::RightButton) {
    // if the right button was pressed, then a context menu might be created soon.
    // it is important to update the selection before the context menu is shown to avoid unexpected
    // happenings.
    this->model()->scene.set_selection(this->selected_items());
  }
  // if the left button is pressed, it is important to not update the selection yet because the user
  // might want to start a drag operation into another object's reference-property field.
  // Since a left button press does not spawn a context menu, that's fine.
}

template<typename ItemViewT, typename ItemModelT>
void ManagerItemView<ItemViewT, ItemModelT>::keyPressEvent(QKeyEvent *e)
{
  // workaround for  https://bugreports.qt.io/browse/QTBUG-62283.
  if (e->key() == Qt::Key_Shift) {
    this->setDragEnabled(false);
  }
  ItemViewT::keyPressEvent(e);
}

template<typename ItemViewT, typename ItemModelT>
void ManagerItemView<ItemViewT, ItemModelT>::keyReleaseEvent(QKeyEvent *e)
{
  // workaround for  https://bugreports.qt.io/browse/QTBUG-62283.
  if (e->key() == Qt::Key_Shift) {
    this->setDragEnabled(true);
  }
  ItemViewT::keyReleaseEvent(e);
}

template<typename ItemViewT, typename ItemModelT>
void ManagerItemView<ItemViewT, ItemModelT>::focusInEvent(QFocusEvent *e)
{
  // workaround for  https://bugreports.qt.io/browse/QTBUG-62283.
  this->setDragEnabled(!(QApplication::keyboardModifiers() & Qt::ShiftModifier));
  ItemViewT::focusInEvent(e);
}

template<typename ItemViewT, typename ItemModelT>
std::set<AbstractPropertyOwner*> ManagerItemView<ItemViewT, ItemModelT>::selected_items() const
{
  const auto get_object = [this](const QModelIndex& index) {
    return &model()->item_at(index);
  };

  const auto selected_indexes = this->selectionModel()->selectedIndexes();
  return ::transform<AbstractPropertyOwner*, std::set>(selected_indexes, get_object);
}

template class ManagerItemView<QListView, ListAdapter<Style>>;
template class ManagerItemView<QTreeView, ObjectTreeAdapter>;

}  // namespace omm
