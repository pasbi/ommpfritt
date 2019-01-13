#pragma once

#include <list>
#include "commands/command.h"
#include "geometry/point.h"
#include "objects/path.h"

namespace omm
{

class Path;
class ModifyPointsCommand : public Command
{
public:
  ModifyPointsCommand(const std::map<Path*, std::map<Point*, Point>>& points);
  void undo() override;
  void redo() override;
  int id() const override;
  bool mergeWith(const QUndoCommand* command) override;

private:
  std::map<Path*, std::map<Point*, Point>> m_data;
  Path::InterpolationMode m_old_interpolation_mode;
  void swap();
};

}  // namespace
