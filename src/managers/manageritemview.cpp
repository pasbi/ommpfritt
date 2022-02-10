#include "managers/manageritemview.h"

#include "commands/propertycommand.h"
#include "commands/removecommand.h"
#include "properties/referenceproperty.h"
#include "renderers/style.h"
#include <QApplication>
#include <QContextMenuEvent>
#include <QMenu>
#include <QMessageBox>

#include "main/application.h"
#include "managers/objectmanager/objecttreeview.h"
#include "managers/stylemanager/stylelistview.h"
#include "scene/scene.h"

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
void ManagerItemView<ItemViewT, ItemModelT>::mousePressEvent(QMouseEvent* e)
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
void ManagerItemView<ItemViewT, ItemModelT>::keyPressEvent(QKeyEvent* e)
{
  // workaround for  https://bugreports.qt.io/browse/QTBUG-62283.
  if (e->key() == Qt::Key_Shift) {
    this->setDragEnabled(false);
  } else {
    switch (e->key()) {
    case Qt::Key_Left:  // NOLINT(bugprone-branch-clone)
      [[fallthrough]];
    case Qt::Key_Right:
      [[fallthrough]];
    case Qt::Key_Up:
      [[fallthrough]];
    case Qt::Key_Down:
      // use the QAbstractItemView key press handler only for selected keys.
      ItemViewT::keyPressEvent(e);
      return;
    default:
      // don't use the QAbstractItemView key press handler for all other keys.
      // They break many keybindings.
      // NOLINTNEXTLINE(bugprone-parent-virtual-call)
      QWidget::keyPressEvent(e);
      return;
    }
  }
}

template<typename ItemViewT, typename ItemModelT>
void ManagerItemView<ItemViewT, ItemModelT>::keyReleaseEvent(QKeyEvent* e)
{
  // workaround for  https://bugreports.qt.io/browse/QTBUG-62283.
  if (e->key() == Qt::Key_Shift) {
    this->setDragEnabled(true);
  }
  ItemViewT::keyReleaseEvent(e);
}

template<typename ItemViewT, typename ItemModelT>
void ManagerItemView<ItemViewT, ItemModelT>::focusInEvent(QFocusEvent* e)
{
  // workaround for  https://bugreports.qt.io/browse/QTBUG-62283.
  this->setDragEnabled(!(QApplication::keyboardModifiers() & Qt::ShiftModifier));
  ItemViewT::focusInEvent(e);
}

template<typename ItemViewT, typename ItemModelT>
std::set<AbstractPropertyOwner*> ManagerItemView<ItemViewT, ItemModelT>::selected_items() const
{
  const auto get_object = [this](const QModelIndex& index) {
    return static_cast<AbstractPropertyOwner*>(&model()->item_at(index));
  };
  const auto selected_indexes = this->selectionModel()->selectedIndexes();
  return util::transform<std::set>(selected_indexes, get_object);
}

template class ManagerItemView<QListView, StyleList>;
template class ManagerItemView<QTreeView, ObjectTree>;

}  // namespace omm
