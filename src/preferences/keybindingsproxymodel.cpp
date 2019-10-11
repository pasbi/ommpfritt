#include "preferences/keybindingsproxymodel.h"
#include "keybindings/keybindings.h"
#include <QKeySequence>

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
    const auto* ptr = static_cast<const PreferencesTreeItem*>(index.internalPointer());
    if (ptr->is_group()) {
      return rowCount(mapFromSource(index)) > 0;
    } else {
      const auto* value = static_cast<const PreferencesTreeValueItem*>(ptr);
      const auto name = QString::fromStdString(value->name);
      const auto sequence = QKeySequence(QString::fromStdString(value->value())).toString(QKeySequence::NativeText);
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
