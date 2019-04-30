#include "scene/history/historymodel.h"

#include <QColor>

namespace omm
{

QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
  switch (role) {
  case Qt::DisplayRole:
    if (index.row() == 0) {
      return tr("foundation");
    } else {
      return QString::fromStdString(command(index.row()-1).label());
    }
  case Qt::ForegroundRole: {
    const int d = index.row() - m_undo_stack.index();
    if (d <= 0) {
      return QColor(Qt::black);
    } else {
      return QColor(Qt::gray);
    }
  }
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

void HistoryModel::set_index(const int index)
{
  m_undo_stack.setIndex(index);
}

std::unique_ptr<HistoryModel::Macro> HistoryModel::start_macro(const QString &text)
{
  return std::make_unique<Macro>(text, m_undo_stack);
}

HistoryModel::HistoryModel()
{
  connect(&m_undo_stack, &QUndoStack::indexChanged, [this](int index) {
    const auto before = this->index(std::max(0, index), 0);
    const auto after = this->index(std::min(m_undo_stack.count()-1, index+1), 0);
    Q_EMIT dataChanged(before, after);
  });
}

int HistoryModel::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent)
  assert(!parent.isValid());
  return count()+1;
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
