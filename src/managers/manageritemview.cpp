#include "managers/manageritemview.h"

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
ManagerItemView<ItemViewT, ItemModelT>::~ManagerItemView()
{
}

template<typename ItemViewT, typename ItemModelT>
void ManagerItemView<ItemViewT, ItemModelT>::setSelectionModel(QItemSelectionModel* model)
{
  const auto on_selection_changed = [this](const auto& selected) {
    if (!m_block_selection_change_signal) {
      this->model()->scene.set_selection(this->selected_items());
    }
  };
  this->connect(model, &QItemSelectionModel::selectionChanged, on_selection_changed);
  ItemViewT::setSelectionModel(model);
}

template<typename ItemViewT, typename ItemModelT>
void ManagerItemView<ItemViewT, ItemModelT>::contextMenuEvent(QContextMenuEvent *event)
{
  auto& app = Application::instance();
  auto menus = app.key_bindings.make_menus(app, application_actions());
  if (menus.size() > 0) {
    assert(menus.size() == 1);
    auto menu = std::move(menus.front());

    populate_menu(*menu, this->indexAt(event->pos()));

    menu->move(event->globalPos());
    menu->show();

    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu.release();
    event->accept();
  }
}

template<typename ItemViewT, typename ItemModelT>
ItemModelT* ManagerItemView<ItemViewT, ItemModelT>::model() const
{
  return static_cast<ItemModelT*>(ItemViewT::model());
}

template<typename ItemViewT, typename ItemModelT>
void ManagerItemView<ItemViewT, ItemModelT>::mouseReleaseEvent(QMouseEvent* e)
{
  // see also mousePressEvent
  e->ignore();
  ItemViewT::mouseReleaseEvent(e);
}

template<typename ItemViewT, typename ItemModelT>
void ManagerItemView<ItemViewT, ItemModelT>::mousePressEvent(QMouseEvent* e)
{
  // selection change must be propagated on key release rather than on key press.
  // otherwise, drag'n'dropping items on other items' reference-properties becomes really hard.
  // see also mouseReleaseEvent.
  m_block_selection_change_signal = true;
  ItemViewT::mousePressEvent(e);
  m_block_selection_change_signal = false;
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

template<typename ItemViewT, typename ItemModelT>
std::vector<std::string> ManagerItemView<ItemViewT, ItemModelT>::application_actions() const
{
  return {};
}

template<typename ItemViewT, typename ItemModelT> void
ManagerItemView<ItemViewT, ItemModelT>::populate_menu(QMenu& menu, const QModelIndex& index) const
{
}

template class ManagerItemView<QListView, ListAdapter<Style>>;
template class ManagerItemView<QTreeView, ObjectTreeAdapter>;

}  // namespace omm
