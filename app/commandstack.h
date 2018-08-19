#pragma once

#include <stack>
#include <memory>
#include "command.h"

class CommandStack
{
public:
  CommandStack();
  virtual void submit(std::unique_ptr<Command> command);

  void undo();
  void redo();

  bool can_undo();
  bool can_redo();

private:
  using Stack = std::stack<std::unique_ptr<Command>>;
  Stack m_undo_stack;
  Stack m_redo_stack;

};