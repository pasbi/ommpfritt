#include "commands/joinpointscommand.h"
#include "objects/pathpoint.h"
#include "scene/scene.h"

namespace omm
{

JoinPointsCommand::JoinPointsCommand(Scene& scene, const std::set<PathPoint*>& points)
  : AbstractJoinPointsCommand(QObject::tr("Join Points"), scene, points)
{
}

void JoinPointsCommand::undo()
{
  scene().joined_points() = m_old_forest;
  for (auto* point : points()) {
    point->set_geometry(m_old_positions[point]);
  }
  update_affected_paths();
}

void JoinPointsCommand::redo()
{
  m_old_forest = scene().joined_points();
  const auto joined = scene().joined_points().insert(points());
  const auto new_pos = compute_position(joined);
  for (auto* point : points()) {
    m_old_positions[point] = point->geometry();
    auto geometry = point->geometry();
    geometry.set_position(new_pos);
    point->set_geometry(geometry);
  }
  update_affected_paths();
}

Vec2f JoinPointsCommand::compute_position(const std::set<PathPoint*>& points)
{
  Vec2f pos{0.F, 0.F};
  for (const auto* p : points) {
    pos += p->geometry().position();
  }
  return pos / static_cast<double>(points.size());
}

DisjoinPointsCommand::DisjoinPointsCommand(Scene& scene, const std::set<PathPoint*>& points)
  : AbstractJoinPointsCommand(QObject::tr("Disjoin Points"), scene, points)
{
}

void DisjoinPointsCommand::undo()
{
  scene().joined_points() = m_old_forest;
}

void DisjoinPointsCommand::redo()
{
  m_old_forest = scene().joined_points();
  for (auto* point : points()) {
    scene().joined_points().remove({point});
  }
}

AbstractJoinPointsCommand::AbstractJoinPointsCommand(const QString& label,
                                                     Scene& scene,
                                                     const std::set<PathPoint*>& points)
  : Command(label)
  , m_scene(scene)
  , m_points(points)
{

}

std::set<PathPoint*> AbstractJoinPointsCommand::points() const
{
  return m_points;
}

Scene& AbstractJoinPointsCommand::scene() const
{
  return m_scene;
}

void AbstractJoinPointsCommand::update_affected_paths() const
{
  std::set<Path*> paths;
  for (const auto* point : points()) {
    paths.insert(point->path());
  }
  for (auto* path : paths) {
    path->update();
  }
}

}  // namespace omm
