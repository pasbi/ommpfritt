#pragma once

#include <set>
#include "commands/command.h"
#include "geometry/objecttransformation.h"
#include "geometry/point.h"

namespace omm
{

class Object;
class Path;
class PointsTransformationCommand : public Command
{
public:
  PointsTransformationCommand(const std::set<Path*>& paths, const ObjectTransformation& t);
  void undo() override;
  void redo() override;
  int id() const override;
  bool mergeWith(const QUndoCommand* command) override;

private:
  std::map<Point*, Point> m_alternative_points;
};

}  // namespace omm
