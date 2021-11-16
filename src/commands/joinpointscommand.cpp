#include "commands/joinpointscommand.h"
#include "objects/pathpoint.h"

namespace omm
{

JoinPointsCommand::JoinPointsCommand(const Map& map)
  : Command(QObject::tr("Join Points"))
  , m_points(map)
{
}

void JoinPointsCommand::undo()
{
  for (auto&& [path, points] : m_points) {
    path->m_joined_points = m_old_forest[path];
    for (auto* point : points) {
      point->geometry() = m_old_positions[point];
    }
    path->update();
  }
}

void JoinPointsCommand::redo()
{
  for (auto&& [path, points] : m_points) {
    m_old_forest[path] = path->m_joined_points;
    const auto joined = path->join_points(points);
    const auto new_pos = compute_position(joined);
    for (auto* point : points) {
      m_old_positions[point] = point->geometry();
      point->geometry().set_position(new_pos);
    }
    path->update();
  }
}

Vec2f JoinPointsCommand::compute_position(const std::set<PathPoint*>& points)
{
  Vec2f pos{0.F, 0.F};
  for (const auto* p : points) {
    pos += p->geometry().position();
  }
  return pos / static_cast<double>(points.size());
}

DisjoinPointsCommand::DisjoinPointsCommand(const Map& map)
  : Command(QObject::tr("Disjoin Points"))
  , m_points(map)
{
}

void DisjoinPointsCommand::undo()
{
  for (auto&& [path, points] : m_points) {
    path->m_joined_points = m_old_forest[path];
    path->update();
  }
}

void DisjoinPointsCommand::redo()
{
  for (auto&& [path, point_iterators] : m_points) {
    m_old_forest[path] = path->m_joined_points;
    for (const auto& it : point_iterators) {
      path->disjoin_points(it);
    }
  }
}

}  // namespace omm
