#pragma once

#include "commands/command.h"
#include "objects/path.h"

#include <vector>
#include <map>

namespace omm
{

class JoinPointsCommand : public Command
{
public:
  using Map = std::map<Path*, std::set<PathPoint*>>;
  explicit JoinPointsCommand(const Map& map);
  void undo() override;
  void redo() override;

private:
  const std::map<Path*, std::set<PathPoint*>> m_points;
  std::map<Path*, DisjointSetForest<PathPoint*>> m_old_forest;
  std::map<PathPoint*, Point> m_old_positions;

  static Vec2f compute_position(const std::set<PathPoint*>& points);
};

class DisjoinPointsCommand : public Command
{
public:
  using Map = std::map<Path*, std::set<PathPoint*>>;
  explicit DisjoinPointsCommand(const Map& map);
  void undo() override;
  void redo() override;

private:
  const std::map<Path*, std::set<PathPoint*>> m_points;
  std::map<Path*, DisjointSetForest<PathPoint*>> m_old_forest;
};

}  // namespace omm
