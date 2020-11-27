#pragma once

#include "commands/command.h"
#include "common.h"
#include "geometry/objecttransformation.h"
#include "geometry/point.h"
#include "objects/path.h"

namespace omm
{
class Object;
class Path;
class PointsTransformationCommand : public Command
{
public:
  using Map = std::map<Path::iterator, Point>;
  PointsTransformationCommand(const Map& new_points);
  void undo() override;
  void redo() override;
  [[nodiscard]] int id() const override;
  bool mergeWith(const QUndoCommand* command) override;
  [[nodiscard]] bool is_noop() const override;

private:
  Map m_old_points;
  Map m_new_points;
  static void apply(const Map& map);
  [[nodiscard]] std::set<Path::iterator> affected_points() const;
};

}  // namespace omm
