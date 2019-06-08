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

  /**
   * @brief last_command returns the command which was pushed most recently to the undo stack.
   * @note caution! if the command is still on the stack, modifying it may cause bad things.
   *   There is a good reason Qt decided to return only const-pointers to commands on the command
   *   stack. (https://doc.qt.io/qt-5/qundostack.html#command).
   *   There are goof reasons not to follow this policy here, too. Just be careful.
   *   If the command is not on the stack, it has probably been deleted. The caller is responsible
   *   for ensuring not to access a deleted object.
   * @note this function is merge-proof. if the last command was auto-deleted because its contents
   *   were merged with the previous one, the previous command will be returned.
   * @note undefined behaviour if index is not at top of the stack. index will be at top if a
   *   command was recently pushed and no undo/redo has occured since.
   */
  Command* last_command() const;
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
