#pragma once

#include <QUndoCommand>

namespace omm
{
class Project;
class Scene;

class Command : public QUndoCommand
{
protected:
  Command(const QString& label);

public:
  [[nodiscard]] virtual bool is_noop() const;

protected:
  static constexpr int PROPERTY_COMMAND_ID = 1;
  static constexpr int OBJECTS_TRANSFORMATION_COMMAND_ID = 2;
  static constexpr int POINTS_TRANSFORMATION_COMMAND_ID = 3;
  static constexpr int MODIFY_TANGENTS_COMMAND_ID = 4;
  static constexpr int MOVE_KEYFRAMES_COMMAND_ID = 5;
  static constexpr int CHANGE_NAMED_COLORS_COLOR_COMMAND_ID = 6;
  static constexpr int CHANGE_KEYFRAMES_COMMAND_ID = 7;
  static constexpr int MOVE_NODES_COMMAND_ID = 8;
  static constexpr int COMPOSE_COMMAND_ID = 9;
};

}  // namespace omm
