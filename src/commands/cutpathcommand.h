#pragma once

#include "commands/command.h"
#include <2geom/pathvector.h>

namespace omm
{

class Path;

class CutPathCommand : public Command
{
public:
  explicit CutPathCommand(Path& path, const std::vector<Geom::PathVectorTime>& cuts);
  void undo() override;
  void redo() override;

private:
  Path& m_path;
  const Geom::PathVector m_original;
  const Geom::PathVector m_cut;
};

}  // namespace omm
