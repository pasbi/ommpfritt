#pragma once

#include <memory>
#include "commands/command.h"

namespace omm
{

class Object;

class AddObjectCommand : public Command
{
public:
  AddObjectCommand(Project& project, std::unique_ptr<Object> object);

  void undo() override;
  void redo() override;

private:
  std::unique_ptr<Object> m_owned_object;
  Object& m_object_reference;
};

}  // namespace omm
