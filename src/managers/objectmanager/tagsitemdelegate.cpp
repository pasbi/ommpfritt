#include "managers/objectmanager/tagsitemdelegate.h"

#include <glog/logging.h>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>

#include "scene/objecttreeadapter.h"
#include "managers/objectmanager/objecttreeview.h"
#include "tags/tag.h"
#include "scene/scene.h"
#include "managers/objectmanager/objecttreeselectionmodel.h"

namespace
{

constexpr QSize icon_size()
{
  return QSize(32, 32);
}

}  // namespace

namespace omm
{

TagsItemDelegate::TagsItemDelegate(ObjectTreeView& view, ObjectTreeSelectionModel& selection_model)
  : m_view(view)
  , m_selection_model(selection_model)
{
}

void TagsItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index ) const
{
  painter->save();
  painter->translate(cell_pos(index));

  QPen pen;
  pen.setWidth(2);
  pen.setColor(Qt::black);

  painter->setPen(pen);
  const auto rect = QRect(QPoint(), icon_size());

  const auto& object = m_view.model()->item_at(index);
  for (size_t i = 0; i < object.tags.size(); ++i)
  {
    auto& tag = object.tags.item(i);
    painter->setClipRect(rect);
    tag.icon().paint(painter, rect);
    if (m_selection_model.is_selected(tag)) {
      painter->drawRect(rect);
    }
    painter->translate(icon_size().width(), 0);
  }

  painter->restore();
}

QSize
TagsItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  const int n_tags = m_view.model()->item_at(index).tags.size();
  const int width = n_tags * icon_size().width();
  return QSize(width, icon_size().height());
}

bool TagsItemDelegate::editorEvent( QEvent *event, QAbstractItemModel *model,
                                    const QStyleOptionViewItem &option, const QModelIndex &index )
{
  switch (event->type()) {
  case QEvent::MouseButtonPress:
    return on_mouse_button_press(*static_cast<QMouseEvent*>(event));
  case QEvent::MouseButtonRelease:
    return on_mouse_button_release(*static_cast<QMouseEvent*>(event));
  default:
    return false;
  }
}

bool TagsItemDelegate::on_mouse_button_press(QMouseEvent& event)
{
  if (event.button() != Qt::LeftButton) { return false; }
  Tag* tag = tag_at(event.pos());
  if (tag == nullptr) { return false; }
  const bool is_selected = m_selection_model.is_selected(*tag);

  if (!is_selected) {
    if (event.modifiers() & Qt::ControlModifier) {
      m_selection_model.select(*tag, QItemSelectionModel::Select);
    } else {
      m_selection_model.select(*tag, QItemSelectionModel::ClearAndSelect);
    }
    m_recently_selected_tag = tag;
  } else {
    m_recently_selected_tag = nullptr;
  }
  return true;
}

bool TagsItemDelegate::on_mouse_button_release(QMouseEvent& event)
{
  if (event.button() != Qt::LeftButton) { return false; }
  Tag* tag = tag_at(event.pos());
  if (tag == nullptr) { return false; }
  const bool is_selected = m_selection_model.is_selected(*tag);
  if (!(event.modifiers() & Qt::ControlModifier)) {
    m_selection_model.select(*tag, QItemSelectionModel::ClearAndSelect);
  } else if (m_recently_selected_tag != tag) {
    m_selection_model.select(*tag, QItemSelectionModel::Toggle);
  }
  m_recently_selected_tag = nullptr;

  return true;
}

QPoint TagsItemDelegate::cell_pos(const QModelIndex& index) const
{
  return m_view.visualRect(index).topLeft();
}

Tag* TagsItemDelegate::tag_at(const QPoint& pos) const { return tag_at(m_view.indexAt(pos), pos); }

Tag*
TagsItemDelegate::tag_at(const QModelIndex& index, const QPoint& pos) const
{
  if (!index.isValid()) { return nullptr; }
  const int x = (pos.x() - cell_pos(index).x()) / icon_size().width();
  Object& object = m_view.model()->item_at(index);
  if (x < 0 || x >= object.tags.size()) { return nullptr; }
  return &object.tags.item(x);
}

Tag* TagsItemDelegate::tag_before(const QPoint& pos) const
{
  return tag_before(m_view.indexAt(pos), pos);
}

Tag* TagsItemDelegate::tag_before(const QModelIndex& index, QPoint pos) const
{
  if (!index.isValid()) { return nullptr; }

  pos -= QPoint(cell_pos(index).x(), 0);

  Object& object = m_view.model()->item_at(index);
  auto tags = object.tags.ordered_items();
  const int x = int(double(pos.x()) / icon_size().width() + 0.5) - 1;
  if (x < 0 || tags.size() == 0) {
    return nullptr;
  } else {
    return tags.at(std::min<int>(tags.size() - 1, x));
  }
}


}  // namespace omm
