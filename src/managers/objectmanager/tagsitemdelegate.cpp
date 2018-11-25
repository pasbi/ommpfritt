#include "managers/objectmanager/tagsitemdelegate.h"

#include <glog/logging.h>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>

#include "managers/objectmanager/objecttreeadapter.h"
#include "managers/objectmanager/objecttreeview.h"
#include "tags/tag.h"

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

auto tags(omm::ObjectTreeView& view, const QModelIndex& index)
{
  if (index.isValid()) {
    return view.model().object_at(index).tags();
  } else {
    return std::vector<omm::Tag*>();
  }
}

}  // namespace

namespace omm
{

TagsItemDelegate::TagsItemDelegate(ObjectTreeView& view)
  : m_view(view)
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
  for (const auto& tag : tags(m_view, index))
  {
    tag->icon().paint(painter, rect);
    if (tag->is_selected()) {
      painter->drawRect(rect);
    }
    painter->translate(icon_size().width(), 0);
  }

  painter->restore();
}

QSize
TagsItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  const int n_tags = tags(m_view, index).size();
  const int width = n_tags * icon_size().width();
  return QSize(width, icon_size().height());
}

bool TagsItemDelegate::editorEvent( QEvent *event, QAbstractItemModel *model,
                                    const QStyleOptionViewItem &option, const QModelIndex &index )
{
  if (event->type() == QEvent::MouseButtonPress) {
    auto mouse_event = static_cast<QMouseEvent*>(event);
    const int x = mouse_event->pos().x() - cell_pos(index).x();
    const size_t tag_i = tag_at(x);
    const auto tags = ::tags(m_view, index);
    if (tag_i >= 0 && tag_i < tags.size()) {
      if (!(mouse_event->modifiers() & Qt::ShiftModifier)) {
        m_view.model().scene().clear_selection();
      }
      Tag& tag = *tags[tag_i];
      tag.set_selected(!tag.is_selected());
      event->accept();
      m_view.update();
    }
  }
  return true;
}

QPoint TagsItemDelegate::cell_pos(const QModelIndex& index) const
{
  return m_view.visualRect(index).topLeft();
}


}  // namespace omm
