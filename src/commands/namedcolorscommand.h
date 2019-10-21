#pragma once

#include "commands/command.h"
#include "color/color.h"

namespace omm
{

class NamedColorAddRemoveCommand : public Command
{
protected:
  NamedColorAddRemoveCommand(const std::string& label, const std::string& color_name, const Color& value);
  void add();
  void remove();

private:
  const std::string m_color_name;
  const Color m_value;
};

class AddNamedColorCommand : public NamedColorAddRemoveCommand
{
public:
  explicit AddNamedColorCommand(const std::string& color_name, const Color& value);
  void undo() override { remove(); }
  void redo() override { add(); }
};

class RemoveNamedColorCommand : public NamedColorAddRemoveCommand
{
public:
  explicit RemoveNamedColorCommand(const std::string& color_name);
  void undo() override { add(); }
  void redo() override { remove(); }
};

class ChangeNamedColorNameCommand : public Command
{
public:
  ChangeNamedColorNameCommand(const std::string& old_name, const std::string& new_name);
  void undo() override;
  void redo() override;

private:
  const std::string m_old_name;
  const std::string m_new_name;
};

class ChangeNamedColorColorCommand : public Command
{
public:
  ChangeNamedColorColorCommand(const std::string& name, const Color& color);
  void undo() override;
  void redo() override;

  int id() const override { return Command::CHANGE_NAMED_COLORS_COLOR_COMMAND_ID; }
  bool mergeWith(const QUndoCommand* other) override;
private:
  const std::string m_name;
  const Color m_old_color;
  const Color m_new_color;
};

}  // namespace omm
