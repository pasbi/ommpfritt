#pragma once

#include "common.h"
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
  using Map = std::map<std::pair<Path*, std::size_t>, Point>;
  PointsTransformationCommand(const Map& old_points);
  void undo() override;
  void redo() override;
  int id() const override;
  bool mergeWith(const QUndoCommand* command) override;
  bool is_noop() const override;

private:
  Map m_old_points;
  Map m_new_points;
  void apply(const Map& map);
  std::set<std::pair<Path*, std::size_t>> affected_points() const;
};

}  // namespace omm
