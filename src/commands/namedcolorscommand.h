#pragma once

#include "color/color.h"
#include "commands/command.h"

namespace omm
{
class NamedColorAddRemoveCommand : public Command
{
protected:
  NamedColorAddRemoveCommand(const QString& label, const QString& color_name, const Color& value);
  void add();
  void remove();

private:
  const QString m_color_name;
  const Color m_value;
};

class AddNamedColorCommand : public NamedColorAddRemoveCommand
{
public:
  explicit AddNamedColorCommand(const QString& color_name, const Color& value);
  void undo() override
  {
    remove();
  }
  void redo() override
  {
    add();
  }
};

class RemoveNamedColorCommand : public NamedColorAddRemoveCommand
{
public:
  explicit RemoveNamedColorCommand(const QString& color_name);
  void undo() override
  {
    add();
  }
  void redo() override
  {
    remove();
  }
};

class ChangeNamedColorNameCommand : public Command
{
public:
  ChangeNamedColorNameCommand(const QString& old_name, const QString& new_name);
  void undo() override;
  void redo() override;

private:
  const QString m_old_name;
  const QString m_new_name;
};

class ChangeNamedColorColorCommand : public Command
{
public:
  ChangeNamedColorColorCommand(const QString& name, const Color& color);
  void undo() override;
  void redo() override;

  [[nodiscard]] int id() const override
  {
    return Command::CHANGE_NAMED_COLORS_COLOR_COMMAND_ID;
  }
  bool mergeWith(const QUndoCommand* other) override;

private:
  const QString m_name;
  const Color m_old_color;
  const Color m_new_color;
};

}  // namespace omm
