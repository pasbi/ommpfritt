#include "preferences/keybindingsproxymodel.h"
#include "keybindings/keybindings.h"
#include "preferences/preferencestreeitem.h"
#include <QKeySequence>

namespace omm
{
KeyBindingsProxyModel::KeyBindingsProxyModel(KeyBindings& key_bindings)
    : QSortFilterProxyModel(nullptr)
{
  setSourceModel(&key_bindings);
}

bool KeyBindingsProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
  if (!source_parent.isValid()) {
    return true;
  } else {
    const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    const auto* ptr = static_cast<const PreferencesTreeItem*>(index.internalPointer());
    if (ptr->is_group()) {
      return rowCount(mapFromSource(index)) > 0;
    } else {
      const auto* value = dynamic_cast<const PreferencesTreeValueItem*>(ptr);
      const auto name = value->translated_name();
      const auto sequence = QKeySequence(value->value()).toString(QKeySequence::NativeText);
      const bool name_matches = name.contains(m_action_name_filter, Qt::CaseInsensitive);
      const QString seq = this->m_action_sequence_filter.toString(QKeySequence::NativeText);
      const bool sequence_matches = sequence.contains(seq, Qt::CaseInsensitive);
      return (m_action_name_filter.isEmpty() || name_matches)
             && (m_action_sequence_filter.isEmpty() || sequence_matches);
    }
  }
}

void KeyBindingsProxyModel::set_action_name_filter(const QString& action_name)
{
  m_action_name_filter = action_name;
  invalidate();
}

void KeyBindingsProxyModel::set_action_sequence_filter(const QKeySequence& sequence)
{
  m_action_sequence_filter = sequence;
  invalidate();
}

}  // namespace omm
