#include "commands/composecommand.h"

namespace omm
{

ComposeCommand::ComposeCommand(const QString& label, std::vector<std::unique_ptr<Command>>&& commands)
  : Command(label)
  , m_commands(std::move(commands))
{
}

void ComposeCommand::redo()
{
  for (auto& command : m_commands) {
    command->redo();
  }
}

void ComposeCommand::undo()
{
  for (auto it = m_commands.rbegin(); it != m_commands.rend(); ++it) {
    (*it)->undo();
  }
}

int ComposeCommand::id() const
{
  return Command::COMPOSE_COMMAND_ID;
}

void ComposeCommand::set_commands(std::vector<std::unique_ptr<Command>>&& commands)
{
  m_commands = std::move(commands);
}

}  // namespace omm
