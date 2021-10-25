#include "scene/history/historymodel.h"
#include "commands/command.h"
#include "scene/history/macro.h"
#include "logging.h"
#include <QColor>

namespace omm
{
HistoryModel::HistoryModel()
{
  connect(&m_undo_stack, &QUndoStack::indexChanged, this, [this](int index) {
    const auto before = this->index(std::max(0, index), 0);
    const auto after = this->index(std::min(m_undo_stack.count() - 1, index + 1), 0);
    Q_EMIT dataChanged(before, after);
  });
  connect(&m_undo_stack, &QUndoStack::indexChanged, this, &HistoryModel::index_changed);
}

QVariant HistoryModel::data(const QModelIndex& index, int role) const
{
  const auto decorate_name = [this](QString name, int row) {
    if (row == m_saved_index) {
      name += tr(" #");
    }
    if (row == m_undo_stack.index()) {
      name += tr(" <-");
    }
    return name;
  };

  const auto row_name = [this](int row) {
    if (row == 0) {
      return tr("foundation");
    } else {
      return m_undo_stack.command(row - 1)->actionText();
    }
  };

  switch (role) {
  case Qt::DisplayRole:
    return decorate_name(row_name(index.row()), index.row());
  default:
    return QVariant();
  }
}

void HistoryModel::push(std::unique_ptr<Command> command)
{
  if (m_saved_index > m_undo_stack.index()) {
    m_saved_index = -1;
  }
  const auto n = count();
  beginInsertRows(QModelIndex(), n, n);
  m_undo_stack.push(command.release());
  endInsertRows();
}

int HistoryModel::count() const
{
  return m_undo_stack.count();
}

void HistoryModel::set_index(const int index)
{
  m_undo_stack.setIndex(index);
}

bool HistoryModel::has_pending_changes() const
{
  return m_saved_index != m_undo_stack.index();
}

void HistoryModel::reset()
{
  beginResetModel();
  m_undo_stack.clear();
  endResetModel();
}

Command* HistoryModel::last_command() const
{
  if (const auto n = count(); n == 0) {
    return nullptr;
  } else {
    const auto* cmd = m_undo_stack.command(n - 1);
    if (cmd->childCount() > 0) {
      // macro
      return nullptr;
    } else {
      // This is dangerous, I know. But there's no other way around that limitation from QUndoStack.
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
      return const_cast<Command*>(dynamic_cast<const Command*>(cmd));
    }
  }
}

void HistoryModel::make_last_command_obsolete()
{
  if (auto* cmd = last_command(); cmd) {
    return cmd->setObsolete(true);
  }
}

bool HistoryModel::last_command_is_noop()
{
  if (const auto* cmd = last_command(); cmd) {
    return cmd->is_noop();
  } else {
    return false;
  }
}

void HistoryModel::set_saved_index()
{
  const QModelIndex old_index = this->index(m_saved_index, 0);
  m_saved_index = m_undo_stack.index();
  const QModelIndex new_index = this->index(m_saved_index, 0);
  Q_EMIT dataChanged(old_index, old_index);
  Q_EMIT dataChanged(new_index, new_index);
}

int HistoryModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  assert(!parent.isValid());
  return count() + 1;
}

void HistoryModel::undo()
{
  m_undo_stack.undo();
}

void HistoryModel::redo()
{
  m_undo_stack.redo();
}

std::unique_ptr<Macro> HistoryModel::start_macro(const QString& text)
{
  return std::make_unique<Macro>(text, m_undo_stack);
}

std::unique_ptr<Macro> HistoryModel::start_remember_selection_macro(const QString& text,
                                                                    Scene& scene)
{
  return std::make_unique<RememberSelectionMacro>(scene, text, m_undo_stack);
}

}  // namespace omm
