#pragma once

#include "commands/command.h"
#include <deque>
#include <memory>
#include "path/pathview.h"

namespace omm
{

class PathPoint;
class Point;

class ModifyPointsCommand : public Command
{
public:
  ModifyPointsCommand(const std::map<PathPoint*, Point>& points);
  void redo() override;
  void undo() override;
  [[nodiscard]] int id() const override;
  bool mergeWith(const QUndoCommand* command) override;
  [[nodiscard]] bool is_noop() const override;

private:
  std::map<PathPoint*, Point> m_data;
  void exchange();
};

}  // namespace omm
