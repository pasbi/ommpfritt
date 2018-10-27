#include "commandstack.h"
#include <assert.h>
#include "glog/logging.h"

omm::CommandStack::CommandStack()
  : m_has_pending_changes(false)
{

}

void omm::CommandStack::submit(std::unique_ptr<omm::Command> command)
{
  m_redo_stack = Stack(); // clear redo stack
  command->redo();

  if (m_undo_stack.empty()) {
    m_undo_stack.push(std::move(command));
  } else {
    // try to merge with last command
    auto& last_command = m_undo_stack.top();
    auto unmerged_command = last_command->merge(std::move(command));
    if (unmerged_command) {
      // command was not completely merged.
      m_undo_stack.push(std::move(unmerged_command));
      m_has_pending_changes = true;
    } else {
      // command was completely merged
    }
  }
}

namespace {

template<typename T>
omm::Command& swap_stack(T& s1, T& s2)
{
  std::unique_ptr<omm::Command> command = std::move(s1.top());
  assert(command);
  s1.pop();
  omm::Command& ref = *command;
  s2.push(std::move(command));
  return ref;
}

}  // namespace

void omm::CommandStack::undo()
{
  if (can_undo()) {
    Command& command = swap_stack(m_undo_stack, m_redo_stack);
    LOG(INFO) << "undo <" << command.label() << ">.";
    command.undo();
    m_has_pending_changes = true;
  } else {
    LOG(FATAL) << "Called undo on empty undo-stack.";
  }
}

void omm::CommandStack::redo()
{
  if (can_redo()) {
    Command& command = swap_stack(m_redo_stack, m_undo_stack);
    LOG(INFO) << "redo <" << command.label() << ">.";
    command.redo();
    m_has_pending_changes = true;
  } else {
    LOG(FATAL) << "Called redo on empty redo-stack.";
  }
}

bool omm::CommandStack::can_undo()
{
  return !m_undo_stack.empty();
}

bool omm::CommandStack::can_redo()
{
  return !m_redo_stack.empty();
}

void omm::CommandStack::set_has_no_pending_changes()
{
  m_has_pending_changes = false;
}

bool omm::CommandStack::has_pending_changes() const
{
  return m_has_pending_changes;
}
