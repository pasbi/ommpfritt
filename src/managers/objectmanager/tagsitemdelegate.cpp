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
  if (event->type() == QEvent::MouseButtonPress)
  {
    auto mouse_event = static_cast<QMouseEvent*>(event);
    const int x = mouse_event->pos().x() - cell_pos(index).x();
    Tag* tag = tag_at(index, mouse_event->pos());
    if (tag != nullptr) {
      QItemSelectionModel::SelectionFlags command = QItemSelectionModel::NoUpdate;
      if (mouse_event->buttons() & Qt::LeftButton) {
        command = QItemSelectionModel::Toggle;
        if (!(mouse_event->modifiers() & Qt::ControlModifier)) {
          command |= QItemSelectionModel::Clear;
        }
      } else if (mouse_event->buttons() & Qt::RightButton) {
        if (m_selection_model.is_selected(*tag)) {
          command = QItemSelectionModel::NoUpdate;
        } else {
          command = QItemSelectionModel::ClearAndSelect;
        }
      }
      m_selection_model.select(*tag, command);
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

Tag* TagsItemDelegate::tag_at(const QModelIndex& index, const QPoint& pos) const
{
  if (!index.isValid()) { return nullptr; }

  const int x = (pos.x() - cell_pos(index).x()) / icon_size().width();
  Object& object = m_view.model()->item_at(index);
  if (x < 0 || x >= object.tags.size()) { return nullptr; }
  return &object.tags.item(x);
}


}  // namespace omm
