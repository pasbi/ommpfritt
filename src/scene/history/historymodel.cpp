#include "scene/history/historymodel.h"
#include "logging.h"
#include <QColor>

namespace omm
{

QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
  const auto decorate_name = [this](const QString& name, int row) {
    if (row == m_saved_index) {
      return name + tr(" #");
    } else {
      return name;
    }
  };

  const auto row_name = [this](int row) {
    if (row == 0) {
      return tr("foundation");
    } else {
      return QString::fromStdString(command(row-1).label());
    }
  };

  const auto row_color = [this](int row) {
    const int d = row - m_undo_stack.index();
    if (d <= 0) {
      return QColor(Qt::black);
    } else {
      return QColor(Qt::gray);
    }
  };

  switch (role) {
  case Qt::DisplayRole:
    return decorate_name(row_name(index.row()), index.row());
  case Qt::ForegroundRole:
    return row_color(index.row());
  }
  return QVariant();
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

const Command &HistoryModel::command(int index) const
{
  return static_cast<const Command&>(*m_undo_stack.command(index));
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

void HistoryModel::set_saved_index()
{
  const QModelIndex old_index = this->index(m_saved_index, 0);
  m_saved_index = m_undo_stack.index();
  const QModelIndex new_index = this->index(m_saved_index, 0);
  Q_EMIT dataChanged(old_index, old_index);
  Q_EMIT dataChanged(new_index, new_index);
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
  connect(&m_undo_stack, SIGNAL(indexChanged(int)), this, SIGNAL(index_changed()));
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
