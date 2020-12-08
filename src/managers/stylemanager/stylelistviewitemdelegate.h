#pragma once

#include <QStyledItemDelegate>

namespace omm
{
class StyleListView;

class StyleListViewItemDelegate : public QStyledItemDelegate
{
private:
  StyleListView& m_view;
  const QSize m_icon_size;
  int m_text_height = 0;

public:
  explicit StyleListViewItemDelegate(omm::StyleListView& view, const QSize& icon_size);
  void
  setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
  void paint(QPainter* painter,
             const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;
  [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem& option,
                               const QModelIndex& index) const override;
  [[nodiscard]] QString displayText(const QVariant& value, const QLocale& locale) const override;
  void updateEditorGeometry(QWidget* editor,
                            const QStyleOptionViewItem& option,
                            const QModelIndex& index) const override;
  void set_text_height(int text_height);

private:
  [[nodiscard]] static QString display_text(const QModelIndex& index);
};

}  // namespace omm
