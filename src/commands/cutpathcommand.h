#pragma once

#include "commands/modifysegmentscommand.h"
#include <2geom/pathvector.h>

namespace omm
{

class Path;

class CutPathCommand : public ModifySegmentsCommand
{
protected:
  explicit CutPathCommand(const QString& label, Path& path, const std::vector<Geom::PathVectorTime>& cuts);
public:
  explicit CutPathCommand(Path& path, const std::vector<Geom::PathVectorTime>& cuts);
};

}  // namespace omm
