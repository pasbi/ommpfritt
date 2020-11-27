#include "managers/stylemanager/stylelistviewitemdelegate.h"
#include "managers/stylemanager/stylelistview.h"

namespace omm
{
StyleListViewItemDelegate::StyleListViewItemDelegate(omm::StyleListView& view,
                                                     const QSize& icon_size)
    : m_view(view), m_icon_size(icon_size)
{
}

void StyleListViewItemDelegate::setModelData(QWidget* editor,
                                             QAbstractItemModel* model,
                                             const QModelIndex& index) const
{
  QStyledItemDelegate::setModelData(editor, model, index);
  m_view.update_text_height();
}

void StyleListViewItemDelegate::paint(QPainter* painter,
                                      const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const
{
  if (!!(option.state & QStyle::State_Selected)) {
    const auto group = [widget = option.widget]() {
      if (widget == nullptr) {
        return QPalette::Active;
      } else if (!widget->isEnabled()) {
        return QPalette::Disabled;
      } else if (widget->window()->isActiveWindow()) {
        return QPalette::Active;
      } else {
        return QPalette::Inactive;
      }
    }();
    painter->fillRect(option.rect, option.palette.color(group, QPalette::Highlight));
  }

  const auto icon = index.data(Qt::DecorationRole).value<QIcon>();
  painter->drawPixmap(option.rect.topLeft(), icon.pixmap(m_icon_size));

  const auto text = this->display_text(index);
  const QRect text_rect(QPoint(option.rect.left(), option.rect.bottom() - m_text_height),
                        QSize(option.rect.width(), m_text_height));

  painter->drawText(text_rect, omm::StyleListView::text_flags, text);
}

QSize StyleListViewItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const
{
  Q_UNUSED(option);
  Q_UNUSED(index);
  return QSize(m_icon_size.width(), m_icon_size.height() + m_text_height);
}

QString StyleListViewItemDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
  Q_UNUSED(value)
  Q_UNUSED(locale)
  return "";
}

void StyleListViewItemDelegate::updateEditorGeometry(QWidget* editor,
                                                     const QStyleOptionViewItem& option,
                                                     const QModelIndex& index) const
{
  Q_UNUSED(index);
  editor->resize(option.rect.width(), m_text_height);
  editor->move(option.rect.left(), option.rect.bottom() - m_text_height);
}

void StyleListViewItemDelegate::set_text_height(int text_height)
{
  m_text_height = text_height;
}

QString StyleListViewItemDelegate::display_text(const QModelIndex& index) const
{
  return index.data(Qt::DisplayRole).toString();
}

}  // namespace omm
