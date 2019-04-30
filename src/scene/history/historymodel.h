#pragma once

#include <QAbstractListModel>
#include "commands/command.h"
#include <memory>

namespace omm
{

class HistoryModel : public QAbstractListModel
{
public:
  HistoryModel();
  int rowCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;

  void push(std::unique_ptr<Command> command);
  void undo();
  void redo();
  int count() const;
  const Command& command(int index) const;
  void set_index(const int index);

  class Macro
  {
  public:
    explicit Macro(const QString& text, QUndoStack& stack);
    ~Macro();
  private:
    QUndoStack& m_undo_stack;
  };

  [[nodiscard]] std::unique_ptr<Macro> start_macro(const QString& text);

private:
  QUndoStack m_undo_stack;
};

}  // namespace omm
