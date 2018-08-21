#pragma once

#include <stack>
#include <memory>
#include "command.h"

namespace omm {

class CommandStack
{
public:
  CommandStack();
  virtual void submit(std::unique_ptr<Command> command);

  void undo();
  void redo();

  bool can_undo();
  bool can_redo();

  bool has_pending_changes() const;

protected:
  void set_has_no_pending_changes();

private:
  using Stack = std::stack<std::unique_ptr<Command>>;
  Stack m_undo_stack;
  Stack m_redo_stack;

  bool m_has_pending_changes;


};

}  // namespace omm