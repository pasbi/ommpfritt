#pragma once

#include "commands/command.h"
#include "objects/path.h"
#include "scene/disjointpathpointsetforest.h"

#include <vector>
#include <map>

namespace omm
{

class AbstractJoinPointsCommand : public Command
{
public:
  explicit AbstractJoinPointsCommand(const QString& label, Scene& scene, const std::set<PathPoint*>& points);

protected:
  std::set<PathPoint*> points() const;
  Scene& scene() const;
  void update_affected_paths() const;

private:
  Scene& m_scene;
  const std::set<PathPoint*> m_points;
};

class JoinPointsCommand : public AbstractJoinPointsCommand
{
public:
  explicit JoinPointsCommand(Scene& scene, const std::set<PathPoint*>& map);
  void undo() override;
  void redo() override;

private:
  DisjointPathPointSetForest m_old_forest;
  std::map<PathPoint*, Point> m_old_positions;

  static Vec2f compute_position(const std::set<PathPoint*>& points);
};

class DisjoinPointsCommand : public AbstractJoinPointsCommand
{
public:
  explicit DisjoinPointsCommand(Scene& scene, const std::set<PathPoint*>& map);
  void undo() override;
  void redo() override;

private:
  DisjointPathPointSetForest m_old_forest;
};

}  // namespace omm
