#pragma once

#include "commands/composecommand.h"
#include <2geom/pathvector.h>

namespace omm
{

class PathObject;

class CutPathCommand : public ComposeCommand
{
protected:
  explicit CutPathCommand(const QString& label, PathObject& path, const std::vector<Geom::PathVectorTime>& cuts);

public:
  explicit CutPathCommand(PathObject& path, const std::vector<Geom::PathVectorTime>& cuts);
};

}  // namespace omm
