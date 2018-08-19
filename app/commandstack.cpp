#include "commandstack.h"

CommandStack::CommandStack()
{

}

void CommandStack::submit(std::unique_ptr<Command> command)
{
  m_redo_stack = Stack(); // clear redo stack
  command->redo();

  if (m_undo_stack.empty()) {
    m_undo_stack.push(std::move(command));
  } else {
    // try to merge with last command
    auto& last_command = m_undo_stack.top();
    auto merged_command = last_command->merge(std::move(command));
    if (merged_command) {
      // command was not completely merged.
      m_undo_stack.push(std::move(command));
    } else {
      // command was completely merged
    }
  }
}

namespace {

template<typename T>
Command& swap_stack(T& s1, T& s2)
{
  std::unique_ptr<Command> command = std::move(s1.top());
  s1.pop();
  Command& ref = *command;
  s2.push(std::move(command));
  return ref;
}

}  // namespace

void CommandStack::undo()
{
  if (can_undo())
  {
    Command& command = swap_stack(m_undo_stack, m_redo_stack);
    command.undo();
  }
}

void CommandStack::redo()
{
  if (can_redo())
  {
    Command& command = swap_stack(m_redo_stack, m_undo_stack);
    command.redo();
  }
}

bool CommandStack::can_undo()
{
  return !m_undo_stack.empty();
}

bool CommandStack::can_redo()
{
  return !m_redo_stack.empty();
}

