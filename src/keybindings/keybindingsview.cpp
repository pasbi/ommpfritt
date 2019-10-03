#include "keybindings/keybindingsview.h"
#include "keybindings/keybindings.h"
#include "logging.h"
#include "widgets/keysequenceedit.h"

namespace omm
{

QWidget* KeySequenceItemDelegate::createEditor( QWidget* parent,
                                                const QStyleOptionViewItem&,
                                                const QModelIndex&) const
{
  return std::make_unique<KeySequenceEdit>(parent).release();
}

void KeySequenceItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  auto* sequence_edit = static_cast<KeySequenceEdit*>(editor);
  sequence_edit->set_default_key_sequence(index.data(KeyBindings::DEFAULT_KEY_SEQUENCE_ROLE)
                                          .value<QKeySequence>());
  sequence_edit->set_key_sequence(index.data(Qt::EditRole).value<QKeySequence>());
}

void KeySequenceItemDelegate::setModelData( QWidget *editor, QAbstractItemModel *model,
                                            const QModelIndex &index ) const
{
  auto* sequence_edit =static_cast<KeySequenceEdit*>(editor);
  const bool s = model->setData(index, sequence_edit->key_sequence(), Qt::EditRole);
  assert(s);
}

KeyBindingsView::KeyBindingsView(QAbstractItemModel &key_bindings)
  : m_sequence_column_delegate(std::make_unique<KeySequenceItemDelegate>())
{
  setModel(&key_bindings);
  setItemDelegateForColumn(KeyBindings::SEQUENCE_COLUMN, m_sequence_column_delegate.get());
  for (int i = 0; i < 3; ++i) {
    resizeColumnToContents(i);
  }
  setSelectionMode(QAbstractItemView::NoSelection);
}

}  // namespace omm
