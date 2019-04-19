#include "scene/history/historymodel.h"

namespace omm
{

QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
  switch (role) {
  case Qt::DisplayRole:
    return QString::fromStdString(command(index.row()).label());
  }
  return QVariant();
}

void HistoryModel::push(std::unique_ptr<Command> command)
{
  const auto n = count();
  beginInsertRows(QModelIndex(), n, n);
  m_undo_stack.push(command.release());
  endInsertRows();
}

int HistoryModel::count() const
{
  return m_undo_stack.count();
}

const Command &HistoryModel::command(int index) const
{
  return static_cast<const Command&>(*m_undo_stack.command(index));
}

std::unique_ptr<HistoryModel::Macro> HistoryModel::start_macro(const QString &text)
{
  return std::make_unique<Macro>(text, m_undo_stack);
}

int HistoryModel::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent)
  assert(!parent.isValid());
  return count();
}

void HistoryModel::undo()
{
  m_undo_stack.undo();
}

void HistoryModel::redo()
{
  m_undo_stack.redo();
}

HistoryModel::Macro::Macro(const QString &text, QUndoStack &stack)
  : m_undo_stack(stack)
{
  m_undo_stack.beginMacro(text);
}

HistoryModel::Macro::~Macro()
{
  m_undo_stack.endMacro();
}

}  // namespace omm
