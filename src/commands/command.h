#pragma once

#include <QUndoCommand>

namespace omm
{

class Project;
class Scene;

class Command : public QUndoCommand
{
protected:
  Command(const std::string& label);

protected:
  static constexpr int PROPERTY_COMMAND_ID = 1;
};

}  // namespace omm
