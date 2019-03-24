#include "keybindings/keybindingstable.h"
#include "keybindings/keybindings.h"
#include <glog/logging.h>

namespace omm
{

QWidget* KeySequenceItemDelegate::createEditor( QWidget* parent,
                                                const QStyleOptionViewItem&,
                                                const QModelIndex&) const
{
  return std::make_unique<QKeySequenceEdit>(parent).release();
}

void KeySequenceItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  auto* sequence_edit =static_cast<QKeySequenceEdit*>(editor);
  sequence_edit->setKeySequence(index.data(Qt::EditRole).value<QKeySequence>());
}

void KeySequenceItemDelegate::setModelData( QWidget *editor, QAbstractItemModel *model,
                                            const QModelIndex &index ) const
{
  auto* sequence_edit =static_cast<QKeySequenceEdit*>(editor);
  const bool s = model->setData(index, sequence_edit->keySequence(), Qt::EditRole);
  assert(s);
}

KeyBindingsTable::KeyBindingsTable(KeyBindings& key_bindings)
  : m_sequence_column_delegate(std::make_unique<KeySequenceItemDelegate>())
{
  setModel(&key_bindings);
  setItemDelegateForColumn(KeyBindings::SEQUENCE_COLUMN, m_sequence_column_delegate.get());
}

QSize KeyBindingsTable::viewportSizeHint() const
{
  LOG(INFO) << QTableView::viewportSizeHint().width();
  int width = 0;
  if (model()) {
    for (int i = 0; i < model()->columnCount(); ++i) {
      width += QAbstractItemView::sizeHintForColumn(i);
    }
  }
  QSize size_hint = QTableView::viewportSizeHint();
  size_hint.setWidth(width);
  return size_hint;
}

}  // namespace omm
