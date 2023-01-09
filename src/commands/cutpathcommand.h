#pragma once

#include "commands/composecommand.h"
#include <2geom/pathvector.h>
#include <set>

namespace omm
{

class PathPoint;
class PathObject;

class CutPathCommand : public ComposeCommand
{
protected:
  explicit CutPathCommand(const QString& label, PathObject& path, const std::vector<Geom::PathVectorTime>& cuts);

public:
  explicit CutPathCommand(PathObject& path, const std::vector<Geom::PathVectorTime>& cuts);
  const std::set<PathPoint*>& new_points() const noexcept;

private:
  std::set<PathPoint*> m_new_points;
};

}  // namespace omm
