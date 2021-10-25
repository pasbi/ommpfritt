#include "managers/objectmanager/objecttreeview.h"

#include "commands/addcommand.h"
#include "commands/propertycommand.h"
#include "commands/removecommand.h"
#include "common.h"
#include "main/application.h"
#include "mainwindow/mainwindow.h"
#include "managers/objectmanager/objectdelegate.h"
#include "managers/objectmanager/objectquickaccessdelegate.h"
#include "managers/objectmanager/tagsitemdelegate.h"
#include "menuhelper.h"
#include "properties/referenceproperty.h"
#include "renderers/style.h"
#include "scene/contextes.h"
#include "scene/mailbox.h"
#include "scene/propertyownermimedata.h"
#include "scene/scene.h"
#include "tags/tag.h"

#include <QApplication>
#include <QDrag>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTimer>
#include <memory>

namespace
{
QItemSelectionModel::SelectionFlag get_selection_flag(const QMouseEvent& event)
{
  return ((event.modifiers() & Qt::ControlModifier) != 0u) ? QItemSelectionModel::Deselect
                                                           : QItemSelectionModel::Select;
}

}  // namespace

namespace omm
{
ObjectTreeView::ObjectTreeView(ObjectTree& model)
    : ManagerItemView(model),
      m_selection_model(std::make_unique<ObjectTreeSelectionModel>(model).release()),
      m_object_quick_access_delegate(std::make_unique<ObjectQuickAccessDelegate>(*this)),
      m_tags_item_delegate(std::make_unique<TagsItemDelegate>(*this, *m_selection_model, 2)),
      m_model(model)
{
  static constexpr int AUTO_EXPAND_DELAY_MS = 200;
  setItemDelegateForColumn(1, m_object_quick_access_delegate.get());
  header()->setSectionResizeMode(1, QHeaderView::Fixed);
  header()->resizeSection(1, row_height);

  setItemDelegateForColumn(2, m_tags_item_delegate.get());

  setSelectionModel(m_selection_model.get());
  setSelectionBehavior(QAbstractItemView::SelectItems);
  setAutoExpandDelay(AUTO_EXPAND_DELAY_MS);

  setIconSize(QSize(row_height, row_height));
  connect(&model, &ObjectTree::expand_item, [this](const QModelIndex& index) {
    setExpanded(index, true);
  });

  connect(&scene().mail_box(),
          &MailBox::tag_inserted,
          this,
          &ObjectTreeView::update_tag_column_size);
  connect(&scene().mail_box(),
          &MailBox::tag_removed,
          this,
          &ObjectTreeView::update_tag_column_size);
  connect(&scene().mail_box(),
          &MailBox::scene_reseted,
          this,
          &ObjectTreeView::update_tag_column_size);

  update_tag_column_size();

  auto object_delegate = std::make_unique<ObjectDelegate>(*this, *m_selection_model);
  m_object_delegate = object_delegate.get();
  setItemDelegateForColumn(0, object_delegate.release());
}

ObjectTreeView::~ObjectTreeView() = default;

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
  setDropIndicatorShown(indexAt(e->pos()).column() != ObjectTree::TAGS_COLUMN);
}

QRect ObjectTreeView::rubber_band() const
{
  return QRect(m_rubberband_corner, m_rubberband_origin).normalized();
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

void ObjectTreeView::mousePressEvent(QMouseEvent* e)
{
  m_aborted = false;
  m_mouse_down_index = indexAt(e->pos());
  if (const QModelIndex index = indexAt(e->pos());
      !index.isValid()
      || (index.column() == 2 && m_tags_item_delegate->tag_at(index, e->pos()) == nullptr)) {
    m_rubberband_visible = true;
    m_rubberband_origin = e->pos();
    m_rubberband_corner = e->pos();
    if (!(e->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier))) {
      m_selection_model->clear_selection();
    }
  } else {
    switch (columnAt(e->pos().x())) {
    case 0:
      [[fallthrough]];
    case 2:
      if (e->button() == Qt::LeftButton) {
        m_mouse_press_pos = e->pos();
      }
      ManagerItemView::mousePressEvent(e);
      break;
    case 1:
      // Attempting to drag a cell from this column would always drag another cell since this one
      // cannot be selected. Hence we must disable dragging for that column.
      // Not returning Draggable-Flag in flags() is not enough.
      // It's important to enable dragging if the mouse is released or focus comes from another
      // widget.
      setDragEnabled(false);
      m_object_quick_access_delegate->on_mouse_button_press(*e);
      break;
    }
  }
}

void ObjectTreeView::mouseReleaseEvent(QMouseEvent* e)
{
  // see ObjectTreeView::mousePressEvent case 1
  if (m_rubberband_visible && !m_aborted) {
    m_rubberband_visible = false;
    const auto flag = get_selection_flag(*e);
    const auto rubber_band_indices = indices(rubber_band());
    for (const QModelIndex& index : rubber_band_indices) {
      selectionModel()->select(index, flag);
    }
    m_object_delegate->tmp_selection.clear();
    const auto viewport_indices = indices(viewport()->rect());
    for (const QModelIndex& index : viewport_indices) {
      for (Tag* tag : m_tags_item_delegate->tags(index, rubber_band())) {
        m_selection_model->select(*tag, flag);
      }
    }
    m_tags_item_delegate->rubberband = QRect(-1, -1, 0, 0);
  }
  setDragEnabled(true);
  const int column = indexAt(e->pos()).column();
  ManagerItemView::mouseReleaseEvent(e);
  if (column == ObjectTree::OBJECT_COLUMN || column == ObjectTree::TAGS_COLUMN) {
    model()->scene.set_selection(selected_items());
  }
  m_object_quick_access_delegate->on_mouse_release(*e);
  viewport()->update();
}

void ObjectTreeView::focusInEvent(QFocusEvent* e)
{
  // see ObjectTreeView::mousePressEvent case 1
  setDragEnabled(true);
  ManagerItemView::focusInEvent(e);
}

void ObjectTreeView::paintEvent(QPaintEvent* event)
{
  ManagerItemView::paintEvent(event);
  if (m_rubberband_visible && !m_aborted) {
    QPainter painter(viewport());
    const QRect rect = QRect(m_rubberband_origin, m_rubberband_corner).normalized();
    painter.drawRect(rect);
  }
}

void ObjectTreeView::keyPressEvent(QKeyEvent* event)
{
  if (!m_aborted && event->key() == Qt::Key_Escape) {
    m_aborted = true;
    m_object_delegate->tmp_selection.clear();
    m_tags_item_delegate->rubberband = QRect(-1, -1, 0, 0);
    viewport()->update();
  } else {
    event->ignore();
  }
  ManagerItemView::keyPressEvent(event);
}

QModelIndexList ObjectTreeView::indices(const QRect rect) const
{
  QModelIndex i = indexAt(rect.topLeft());
  QModelIndexList list;
  while (i.isValid() && visualRect(i).intersects(rect)) {
    list.append(i);
    i = indexBelow(i);
  }
  return list;
}

void ObjectTreeView::mouseMoveEvent(QMouseEvent* e)
{
  if (m_rubberband_visible) {
    if (m_aborted) {
      m_rubberband_visible = false;
    } else {
      m_rubberband_corner = e->pos();
      const QRect rubber_band = this->rubber_band();
      const QItemSelectionModel::SelectionFlag selection_flag = get_selection_flag(*e);
      m_object_delegate->tmp_selection = indices(rubber_band);
      m_object_delegate->selection_flag = selection_flag;
      m_tags_item_delegate->rubberband = rubber_band;
      m_tags_item_delegate->selection_flag = selection_flag;
    }
    viewport()->update();
  } else {
    switch (m_mouse_down_index.column()) {
    case ObjectTree::VISIBILITY_COLUMN:
      m_object_quick_access_delegate->on_mouse_move(*e);
      break;
    case ObjectTree::TAGS_COLUMN:
      if ((e->pos() - m_mouse_press_pos).manhattanLength() > QApplication::startDragDistance()) {
        const auto left_button = e->buttons() & Qt::LeftButton;
        if (left_button != 0u) {
          const auto selected_tags = m_selection_model->selected_tags_ordered(model()->scene);
          const auto st_apo = down_cast(selected_tags);
          if (!selected_tags.empty()) {
            auto mime_data = std::make_unique<PropertyOwnerMimeData>(st_apo);
            auto drag = std::make_unique<QDrag>(this);
            drag->setMimeData(mime_data.release());
            // drag->setPixmap()  // TODO
            drag.release()->exec(Qt::CopyAction | Qt::MoveAction | Qt::LinkAction);
          }
          return;
        }
      }
      [[fallthrough]];
    case ObjectTree::OBJECT_COLUMN:
      ManagerItemView::mouseMoveEvent(e);
    }
  }
}

void ObjectTreeView::set_selection(const std::set<AbstractPropertyOwner*>& selected_items)
{
  m_selection_model->set_selection(selected_items);
}

void ObjectTreeView::update_tag_column_size()
{
  const auto n_tags = m_model.max_number_of_tags_on_object();
  const auto tags_width = m_tags_item_delegate->tag_icon_size().width();
  setColumnWidth(ObjectTree::TAGS_COLUMN, static_cast<int>(n_tags * tags_width));
}

Scene& ObjectTreeView::scene() const
{
  return m_model.scene;
}

}  // namespace omm
