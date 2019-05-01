#pragma once

#include <QAbstractListModel>
#include "commands/command.h"
#include <memory>

namespace omm
{

class HistoryModel : public QAbstractListModel
{
  Q_OBJECT
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
  bool has_pending_changes() const;
  void reset();

  /**
   * @brief set_saved_index indicates that the current index is saved.
   * 	Removes save-indication from all other indices.
   */
  void set_saved_index();

  class Macro
  {
  public:
    explicit Macro(const QString& text, QUndoStack& stack);
    ~Macro();
  private:
    QUndoStack& m_undo_stack;
  };

  [[nodiscard]] std::unique_ptr<Macro> start_macro(const QString& text);

Q_SIGNALS:
  void index_changed();

private:
  QUndoStack m_undo_stack;
  int m_saved_index = 0;
};

}  // namespace omm
