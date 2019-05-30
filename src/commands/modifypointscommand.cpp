#include "commands/modifypointscommand.h"
#include "scene/scene.h"

namespace omm
{

ModifyPointsCommand
::ModifyPointsCommand(const std::map<Path*, std::map<Point*, Point>>& points)
  : Command(QObject::tr("ModifyPointsCommand").toStdString()), m_data(points)
{
  assert(points.size() > 0);
}

void ModifyPointsCommand::undo() { swap(); }
void ModifyPointsCommand::redo() { swap(); }
int ModifyPointsCommand::id() const { return Command::MODIFY_TANGENTS_COMMAND_ID; }

void ModifyPointsCommand::swap()
{
  for (auto& [path, points] : m_data) {
    for (auto& [point_ptr, other] : points) {
      point_ptr->swap(other);
    }
    path->on_change(path, Path::POINTS_CHANGED, nullptr, { this });
  }
}

bool ModifyPointsCommand::mergeWith(const QUndoCommand* command)
{
  // merging happens automatically!
  const auto& mtc = static_cast<const ModifyPointsCommand&>(*command);

  if (m_data.size() != mtc.m_data.size()) {
    return false;
  }
  for (auto&& [path, points] : m_data) {
    if (mtc.m_data.count(path) == 0) {
      return false;
    }
    auto& other_points = mtc.m_data.at(path);
    if (points.size() != other_points.size()) {
      return false;
    }
    for (auto&& [point, other] : points) {
      if (other_points.count(point) == 0) {
        return false;
      }
    }
  }
  return true;
}

AbstractPointsCommand::AbstractPointsCommand(
  const std::string& label, const std::map<Path*, std::vector<Path::PointSequence>>& points )
  : Command(label), m_added_points(points)
{
  assert(m_added_points.size() > 0);
}

AbstractPointsCommand::AbstractPointsCommand(
  const std::string& label, const std::map<Path*, std::vector<std::size_t>>& points )
  : Command(label), m_removed_points(points)
{
  assert(m_removed_points.size() > 0);
}

Scene &AbstractPointsCommand::scene() const
{
  Scene* scene = m_added_points.size() > 0 ? m_added_points.begin()->first->scene()
                                           : m_removed_points.begin()->first->scene();
#ifndef NDEBUG
  for (auto&& [path, points] : m_added_points) {
    assert(scene == path->scene());
  }
  for (auto&& [path, points] : m_removed_points) {
    assert(scene == path->scene());
  }
#endif  // NDEBUG
  return *scene;
}

void AbstractPointsCommand::remove()
{
  assert(m_added_points.size() == 0);
  for (auto&& [path, points] : m_removed_points) {
    m_added_points[path] = path->remove_points(points);
    path->update_tangents();
    path->on_change(path, Path::POINTS_CHANGED, nullptr, { this });
  }
  m_removed_points.clear();
  scene().update_tool();
}

bool AbstractPointsCommand::requires_tool_update() const { return true; }

void AbstractPointsCommand::add()
{
  assert(m_removed_points.size() == 0);
  for (auto&& [path, points] : m_added_points) {
    m_removed_points[path] = path->add_points(points);
    path->update_tangents();
    path->on_change(path, Path::POINTS_CHANGED, nullptr, { this });
  }
  m_added_points.clear();
  scene().update_tool();
}

AddPointsCommand::AddPointsCommand(const std::map<Path*, std::vector<Path::PointSequence>>& points)
  : AbstractPointsCommand(QObject::tr("AddPointsCommand").toStdString(), points) {}

void AddPointsCommand::redo() { add(); }
void AddPointsCommand::undo() { remove(); }

RemovePointsCommand::RemovePointsCommand(const std::map<Path*, std::vector<std::size_t>>& points)
  : AbstractPointsCommand(QObject::tr("RemovePointsCommand").toStdString(), points) {}

void RemovePointsCommand::redo() { remove(); }
void RemovePointsCommand::undo() { add(); }


}  // namespace omm
