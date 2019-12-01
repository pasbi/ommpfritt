#include <QIdentityProxyModel>
#include "preferences/uicolors.h"
#include "managers/objectmanager/objecttreeselectionmodel.h"
#include "managers/objectmanager/objectdelegate.h"
#include "common.h"
#include <QPainter>
#include "proxychain.h"
#include "managers/objectmanager/objecttreeview.h"

namespace omm
{


ObjectDelegate::ObjectDelegate(ObjectTreeView& object_tree_view,
               ObjectTreeSelectionModel& selection_model, ProxyChain& proxy_chain)
  : m_object_tree_view(object_tree_view)
  , m_selection_model(selection_model)
  , m_proxy_chain(proxy_chain)
{
}

void ObjectDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QRect rect = option.rect;
  rect.setLeft(0);

  assert(index.isValid());

  painter->save();
  painter->setClipRect(rect);

  { // background
    const bool is_selected = [this, index]() {
      const bool is_selected = m_selection_model.isSelected(m_proxy_chain.mapToChainSource(index));
      const bool is_tmp_selected = ::contains(tmp_selection, index);
      switch (selection_flag) {
      case QItemSelectionModel::Select:
        return is_selected || is_tmp_selected;
      case QItemSelectionModel::Toggle:
        return is_selected != is_tmp_selected;
      case QItemSelectionModel::Deselect:
        return is_selected && !is_tmp_selected;
      default:
        qFatal("Unexpected selection flag");
        Q_UNREACHABLE();
        return false;
      }
    }();

    const QColor bg = is_selected ? ui_color(*option.widget, QPalette::Highlight)
                                  : ui_color(*option.widget, QPalette::Base);

    painter->fillRect(rect, bg);
  }

  const int w = rect.height();

  { // icon
    const QIcon decoration = index.data(Qt::DecorationRole).value<QIcon>();
    const QPixmap pixmap = decoration.pixmap(QSize(w, w), QIcon::Normal, QIcon::On);
    painter->drawPixmap(option.rect.topLeft() + QPointF(0, 0), pixmap, pixmap.rect());
  }

  { // text
    QString text = index.data(Qt::DisplayRole).toString();
    QRect text_rect = option.rect;
    text_rect.setLeft(text_rect.left() + w + 10);
    text = painter->fontMetrics().elidedText(text, Qt::ElideRight, text_rect.width());
    painter->drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, text);
  }

  { // expand-arrow
    if (index.model()->rowCount(index) > 0) {
      const QString hint = m_object_tree_view.isExpanded(index) ? "▼" : "▶";
      const QRect hint_rect(option.rect.left() - w, option.rect.top(), w, w);
      painter->drawText(hint_rect, Qt::AlignCenter, hint);
    }
  }

  painter->restore();
}

}  // namespace omm
