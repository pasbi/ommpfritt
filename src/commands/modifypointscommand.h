#pragma once

#include "commands/command.h"
#include <deque>
#include <memory>

namespace omm
{

class PathPoint;
class Point;

class ModifyPointsCommand : public Command
{
public:
  using ModifiedPointsMap = std::map<PathPoint*, Point>;
  ModifyPointsCommand(ModifiedPointsMap points);
  void redo() override;
  void undo() override;
  [[nodiscard]] int id() const override;
  bool mergeWith(const QUndoCommand* command) override;
  [[nodiscard]] bool is_noop() const override;

private:
  ModifiedPointsMap m_data;
  void exchange();
};

}  // namespace omm
