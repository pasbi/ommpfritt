#include "keybindings/keybindingsproxymodel.h"
#include "keybindings/keybindings.h"

namespace omm
{

KeyBindingsProxyModel::KeyBindingsProxyModel(KeyBindings &key_bindings)
  : QSortFilterProxyModel(nullptr)
{
  setSourceModel(&key_bindings);
}

bool KeyBindingsProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  if (!source_parent.isValid()) {
    return true;
  } else {
    const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    if (static_cast<const KeyBindingTreeItem*>(index.internalPointer())->is_context()) {
      return true;  // never filter context nodes
    } else {
      const auto* key_binding = static_cast<const KeyBinding*>(index.internalPointer());
      const auto name = QString::fromStdString(key_binding->name);
      const auto sequence = key_binding->key_sequence().toString(QKeySequence::NativeText);
      const bool name_matches = name.contains(QString::fromStdString(m_action_name_filter),
                                              Qt::CaseInsensitive);
      const bool sequence_matches = sequence.contains(QString::fromStdString(m_action_sequence_filter),
                                                      Qt::CaseInsensitive);
      return (m_action_name_filter.empty() || name_matches)
          && (m_action_sequence_filter.empty() || sequence_matches);
    }
  }
}

void KeyBindingsProxyModel::set_action_name_filter(const std::string& action_name)
{
  m_action_name_filter = action_name;
  invalidate();
}

void KeyBindingsProxyModel::set_action_sequence_filter(const std::string& action_sequence)
{
  m_action_sequence_filter = action_sequence;
  invalidate();
}

};
