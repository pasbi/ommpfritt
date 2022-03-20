#include "commands/joinpointscommand.h"
#include "objects/pathobject.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "scene/scene.h"

namespace omm
{

JoinPointsCommand::JoinPointsCommand(Scene& scene, const DisjointPathPointSetForest& forest)
  : AbstractJoinPointsCommand(QObject::tr("Join Points"), scene, forest)
{
}

void JoinPointsCommand::undo()
{
  scene().joined_points() = m_old_forest;
  for (const auto& set : forest().sets()) {
    for (auto* point : set) {
      point->set_geometry(m_old_positions[point]);
    }
  }
  update_affected_paths();
}

void JoinPointsCommand::redo()
{
  m_old_forest = scene().joined_points();
  for (const auto& set : forest().sets()) {
    const auto joined = scene().joined_points().insert(set);
    const auto new_pos = compute_position(joined);
    for (auto* point : set) {
      m_old_positions[point] = point->geometry();
      auto geometry = point->geometry();
      geometry.set_position(new_pos);
      point->set_geometry(geometry);
    }
  }
  update_affected_paths();
}

Vec2f JoinPointsCommand::compute_position(const ::transparent_set<PathPoint*>& points)
{
  Vec2f pos{0.F, 0.F};
  for (const auto* p : points) {
    pos += p->geometry().position();
  }
  return pos / static_cast<double>(points.size());
}

DisjoinPointsCommand::DisjoinPointsCommand(Scene& scene, const DisjointPathPointSetForest& forest)
  : AbstractJoinPointsCommand(QObject::tr("Disjoin Points"), scene, forest)
{
}

void DisjoinPointsCommand::undo()
{
  scene().joined_points() = m_old_forest;
}

void DisjoinPointsCommand::redo()
{
  m_old_forest = scene().joined_points();
  for (const auto& set : forest().sets()) {
    for (auto* point : set) {
      scene().joined_points().remove({point});
    }
  }
}

AbstractJoinPointsCommand::AbstractJoinPointsCommand(const QString& label,
                                                     Scene& scene,
                                                     const DisjointPathPointSetForest& forest)
  : Command(label)
  , m_scene(scene)
  , m_forest(forest)
{

}

const DisjointPathPointSetForest& AbstractJoinPointsCommand::forest() const
{
  return m_forest;
}

Scene& AbstractJoinPointsCommand::scene() const
{
  return m_scene;
}

void AbstractJoinPointsCommand::update_affected_paths() const
{
  std::set<PathVector*> path_vectors;
  for (const auto& set : forest().sets()) {
    for (const auto* point : set) {
      path_vectors.insert(point->path_vector());
    }
  }
  for (auto* path_vector : path_vectors) {
    path_vector->path_object()->update();
  }
}

ShareJoinedPointsCommand::ShareJoinedPointsCommand(Scene& scene, PathVector& pv)
    : Command("Join shared points")
    , m_scene(scene)
    , m_pv(pv)
{
}

void ShareJoinedPointsCommand::undo()
{
  m_scene.joined_points() = m_old_scene_joined_points;
  m_pv.unshare_joined_points(std::move(m_old_other_joined_points));
}

void ShareJoinedPointsCommand::redo()
{
  m_old_scene_joined_points = m_scene.joined_points();
  m_old_other_joined_points = m_pv.share_joined_points(m_scene.joined_points());
}

}  // namespace omm
