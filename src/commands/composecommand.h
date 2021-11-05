#pragma once

#include "commands/command.h"
#include <memory>

namespace omm
{

class ComposeCommand : public Command
{
public:
  explicit ComposeCommand(const QString& label, std::vector<std::unique_ptr<Command>>&& commands = {});
  void redo() override;
  void undo() override;
  [[nodiscard]] int id() const override;

protected:
  void set_commands(std::vector<std::unique_ptr<Command>>&& commands);

private:
  std::vector<std::unique_ptr<Command>> m_commands;
};

}  // namespace omm
