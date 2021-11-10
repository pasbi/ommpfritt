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
  using Map = std::map<Path*, std::set<Point*>>;
  explicit JoinPointsCommand(const Map& map);
  void undo() override;
  void redo() override;

private:
  const std::map<Path*, std::set<Point*>> m_points;
  std::map<Path*, DisjointSetForest<Point*>> m_old_forest;
  std::map<Point*, Point> m_old_positions;

  static Vec2f compute_position(const std::set<Point*>& points);
};

class DisjoinPointsCommand : public Command
{
public:
  using Map = std::map<Path*, std::set<Point*>>;
  explicit DisjoinPointsCommand(const Map& map);
  void undo() override;
  void redo() override;

private:
  const std::map<Path*, std::set<Point*>> m_points;
  std::map<Path*, DisjointSetForest<Point*>> m_old_forest;
};

}  // namespace omm
