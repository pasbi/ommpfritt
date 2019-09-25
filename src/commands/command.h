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

public:
  std::string label() const;
  virtual bool is_noop() const { return false; }

protected:
  static constexpr int PROPERTY_COMMAND_ID = 1;
  static constexpr int OBJECTS_TRANSFORMATION_COMMAND_ID = 2;
  static constexpr int POINTS_TRANSFORMATION_COMMAND_ID = 3;
  static constexpr int MODIFY_TANGENTS_COMMAND_ID = 4;
  static constexpr int MOVE_KEYFRAMES_COMMAND_ID = 5;
};

}  // namespace omm
