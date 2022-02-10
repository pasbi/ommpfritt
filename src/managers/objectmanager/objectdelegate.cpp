#include "managers/objectmanager/objectdelegate.h"
#include "common.h"
#include "managers/objectmanager/objecttreeselectionmodel.h"
#include "managers/objectmanager/objecttreeview.h"
#include "preferences/uicolors.h"
#include <QIdentityProxyModel>
#include <QPainter>

namespace omm
{

ObjectDelegate::ObjectDelegate(ObjectTreeView& object_tree_view,
                               ObjectTreeSelectionModel& selection_model)
    : m_object_tree_view(object_tree_view)
    , m_selection_model(selection_model)
{
}

void ObjectDelegate::paint(QPainter* painter,
                           const QStyleOptionViewItem& option,
                           const QModelIndex& index) const
{
  QRect rect = option.rect;
  rect.setLeft(0);

  assert(index.isValid());

  painter->save();
  painter->setClipRect(rect);

  {  // background
    const bool is_selected = [this, index]() {
      const bool is_selected = m_selection_model.isSelected(index);
      const bool is_tmp_selected = tmp_selection.contains(index);
      switch (selection_flag) {
      case QItemSelectionModel::Select:
        return is_selected || is_tmp_selected;
      case QItemSelectionModel::Toggle:
        return is_selected != is_tmp_selected;
      case QItemSelectionModel::Deselect:
        return is_selected && !is_tmp_selected;
      default:
        LFATAL("Unexpected selection flag");
        Q_UNREACHABLE();
        return false;
      }
    }();

    const QColor bg = is_selected ? ui_color(*option.widget, QPalette::Highlight)
                                  : ui_color(*option.widget, QPalette::Base);

    painter->fillRect(rect, bg);
  }

  const int w = rect.height();

  {  // icon
    const auto decoration = index.data(Qt::DecorationRole).value<QIcon>();
    const QPixmap pixmap = decoration.pixmap(QSize(w, w), QIcon::Normal, QIcon::On);
    painter->drawPixmap(option.rect.topLeft() + QPointF(0, 0), pixmap, pixmap.rect());
  }

  {  // text
    static constexpr int LEFT_MARGIN = 10;
    QString text = index.data(Qt::DisplayRole).toString();
    QRect text_rect = option.rect;
    text_rect.setLeft(text_rect.left() + w + LEFT_MARGIN);
    text = painter->fontMetrics().elidedText(text, Qt::ElideRight, text_rect.width());
    painter->drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, text);
  }

  {  // expand-arrow
    if (index.model()->rowCount(index) > 0) {
      const QString hint = m_object_tree_view.isExpanded(index) ? "▼" : "▶";
      const QRect hint_rect(option.rect.left() - w, option.rect.top(), w, w);
      painter->drawText(hint_rect, Qt::AlignCenter, hint);
    }
  }

  painter->restore();
}

}  // namespace omm
