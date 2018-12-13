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

int tag_at(int pos_x)
{
  return pos_x / icon_size().width();
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
    if (m_selection_model.is_selected(index.siblingAtColumn(0), tag)) {
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
  if (event->type() == QEvent::MouseButtonPress)
  {
    auto mouse_event = static_cast<QMouseEvent*>(event);
    const int x = mouse_event->pos().x() - cell_pos(index).x();
    const size_t tag_i = tag_at(x);
    const auto& object = m_view.model()->item_at(index);
    auto& tag = object.tags.item(tag_i);
    const QModelIndex object_index = index.siblingAtColumn(0);
    if (tag_i >= 0 && tag_i < object.tags.size()) {
      QItemSelectionModel::SelectionFlags command = QItemSelectionModel::NoUpdate;
      if (mouse_event->buttons() & Qt::LeftButton) {
        command = QItemSelectionModel::Toggle;
        if (!(mouse_event->modifiers() & Qt::ControlModifier)) {
          command |= QItemSelectionModel::Clear;
        }
      } else if (mouse_event->buttons() & Qt::RightButton) {
        if (m_selection_model.is_selected(object_index, tag)) {
          command = QItemSelectionModel::NoUpdate;
        } else {
          command = QItemSelectionModel::ClearAndSelect;
        }
      }
      m_selection_model.select(object_index, tag, command);
      m_view.update();
      event->ignore();
      return true;
    }
  }
  return false;
}

QPoint TagsItemDelegate::cell_pos(const QModelIndex& index) const
{
  return m_view.visualRect(index).topLeft();
}


}  // namespace omm
