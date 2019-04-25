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
  assert(!source_parent.isValid());
  auto* source_model = sourceModel();
  const auto data = [source_model, source_row](int column) {
    return source_model->data(source_model->index(source_row, column));
  };

  if (!m_context_name_filter.empty()) {
    if (data(KeyBindings::CONTEXT_COLUMN) != QString::fromStdString(m_context_name_filter)) {
      return false;
    }
  }

  if (!m_action_filter.empty()) {
    std::function<bool(const QVariant& v)> matches;
    matches = [this, &matches](const QVariant& candidate) {
      const auto c = QString::fromStdString(m_action_filter);
      switch (candidate.type()) {
      case QVariant::String:
        return candidate.toString().contains(c, Qt::CaseInsensitive);
      case QVariant::KeySequence:
        return matches(candidate.value<QKeySequence>().toString(QKeySequence::NativeText));
      default:
        Q_UNREACHABLE();
      }
    };
    if (matches(data(KeyBindings::NAME_COLUMN))) {
      return true;
    } else if (matches(data(KeyBindings::SEQUENCE_COLUMN))) {
      return true;
    } else {
      return false;
    }
  }

  // both filters are empty. Accept anything.
  return true;
}

void KeyBindingsProxyModel::set_context_filter(const std::string &context_name)
{
  m_context_name_filter = context_name;
  invalidate();
}

void KeyBindingsProxyModel::set_action_filter(const std::string &action_name)
{
  m_action_filter = action_name;
  invalidate();
}

std::vector<std::string> KeyBindingsProxyModel::contextes() const
{
  auto* source_model = sourceModel();
  auto n = source_model->rowCount();

  std::set<std::string> contextes;
  for (int i = 0; i < n; ++i) {
    const auto index = source_model->index(i, KeyBindings::CONTEXT_COLUMN);
    contextes.insert(source_model->data(index).toString().toStdString());
  }

  return std::vector(contextes.begin(), contextes.end());
}

};
