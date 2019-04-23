#include "managers/objectmanager/objecttreeview.h"

#include <memory>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>
#include <QDrag>

#include "menuhelper.h"
#include "scene/contextes.h"
#include "commands/removecommand.h"
#include "commands/addcommand.h"
#include "renderers/style.h"
#include "scene/scene.h"
#include "commands/propertycommand.h"
#include "properties/referenceproperty.h"
#include "common.h"
#include "scene/propertyownermimedata.h"
#include "tags/tag.h"
#include "mainwindow/application.h"
#include "mainwindow/mainwindow.h"
#include <QHeaderView>
#include <QTimer>

namespace omm
{

ObjectTreeView::ObjectTreeView(ObjectTreeAdapter& model)
  : ManagerItemView(model)
  , m_selection_model(std::make_unique<ObjectTreeSelectionModel>(model).release())
  , m_object_quick_access_delegate(std::make_unique<ObjectQuickAccessDelegate>(*this))
  , m_tags_item_delegate(std::make_unique<TagsItemDelegate>(*this, *m_selection_model))
  , m_update_timer(std::make_unique<QTimer>())
  , m_model(model)
{
  setItemDelegateForColumn(1, m_object_quick_access_delegate.get());
  header()->setSectionResizeMode(1, QHeaderView::Fixed);
  header()->resizeSection(1, ObjectQuickAccessDelegate::width);

  setItemDelegateForColumn(2, m_tags_item_delegate.get());

  setSelectionModel(m_selection_model.get());
  setSelectionBehavior(QAbstractItemView::SelectItems);
  setAutoExpandDelay(200);

//  connect(m_update_timer.get(), &QTimer::timeout, [this]() {
//    viewport()->update();
//  });
//  m_update_timer->start(200);
  setIconSize(QSize(24, 24));

}

std::set<AbstractPropertyOwner*> ObjectTreeView::selected_items() const
{
  return ::merge(selected_objects(), selected_tags());
}

std::set<AbstractPropertyOwner*> ObjectTreeView::selected_objects() const
{
  return ManagerItemView::selected_items();
}

std::set<AbstractPropertyOwner*> ObjectTreeView::selected_tags() const
{
  return down_cast(m_selection_model->selected_tags());
}

void ObjectTreeView::handle_drag_event(QDragMoveEvent* e)
{
  m_model.current_tag_predecessor = m_tags_item_delegate->tag_before(e->pos());
  m_model.current_tag = m_tags_item_delegate->tag_at(e->pos());
  setDropIndicatorShown(indexAt(e->pos()).column() != ObjectTreeAdapter::TAGS_COLUMN);
}

void ObjectTreeView::dragEnterEvent(QDragEnterEvent* e)
{
  handle_drag_event(e);
  ManagerItemView::dragEnterEvent(e);
}

void ObjectTreeView::dragMoveEvent(QDragMoveEvent* e)
{
  handle_drag_event(e);
  ManagerItemView::dragMoveEvent(e);
}

void ObjectTreeView::paintEvent(QPaintEvent* e)
{
  const auto n_tags = m_model.max_number_of_tags_on_object();
  const auto tags_width = m_tags_item_delegate->tag_icon_size().width();
  setColumnWidth(ObjectTreeAdapter::TAGS_COLUMN, n_tags * tags_width);
  ManagerItemView<QTreeView, ObjectTreeAdapter>::paintEvent(e);
}

void ObjectTreeView::mousePressEvent(QMouseEvent* e)
{
  switch (columnAt(e->pos().x())) {
    case 0:
    case 2:
      if (e->button() == Qt::LeftButton) {
        m_dragged_index = indexAt(e->pos());
        m_mouse_press_pos = e->pos();
      }
      ManagerItemView::mousePressEvent(e);
      break;
    case 1:
      m_object_quick_access_delegate->on_mouse_button_press(*e);
      break;
  }
}

void ObjectTreeView::mouseMoveEvent(QMouseEvent* e)
{
  if ((e->pos() - m_mouse_press_pos).manhattanLength() > QApplication::startDragDistance()) {
    const auto tag_column = m_dragged_index.column() == ObjectTreeAdapter::TAGS_COLUMN;
    const auto left_button = e->buttons() & Qt::LeftButton;
    if (left_button && tag_column) {
      const auto selected_tags = m_selection_model->selected_tags_ordered(model()->scene);
      const auto st_apo = down_cast(selected_tags);
      if (selected_tags.size() > 0) {
        auto mime_data = std::make_unique<PropertyOwnerMimeData>(st_apo);
        auto drag = std::make_unique<QDrag>(this);
        drag->setMimeData(mime_data.release());
        // drag->setPixmap()  // TODO
        drag.release()->exec(Qt::CopyAction | Qt::MoveAction | Qt::LinkAction);
      }
      return;
    }
  }
  ManagerItemView::mouseMoveEvent(e);
}

void ObjectTreeView::set_selection(const std::set<AbstractPropertyOwner*>& selected_items)
{
  m_selection_model->set_selection(selected_items);
}

Scene &ObjectTreeView::scene() const { return m_model.scene; }


}  // namespace omm
