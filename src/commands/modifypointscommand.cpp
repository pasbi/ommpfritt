#include "commands/modifypointscommand.h"

namespace omm
{

ModifyPointsCommand
::ModifyPointsCommand(const std::map<Path*, std::map<Point*, Point>>& points)
  : Command("ModifyPointsCommand")
  , m_data(points)
{

}

void ModifyPointsCommand::undo()
{
  swap();
  // if (m_path != nullptr) { m_path->set_interpolation_mode(m_old_interpolation_mode); }
}

void ModifyPointsCommand::redo()
{
  swap();
  // if (m_path != nullptr) { m_path->set_interpolation_mode(Path::InterpolationMode::Bezier); }
}

void ModifyPointsCommand::swap()
{
  for (auto [path, points] : m_data) {
    for (auto [point_ptr, other] : points) {
      point_ptr->swap(other);
    }
  }
}

int ModifyPointsCommand::id() const
{
  return Command::MODIFY_TANGENTS_COMMAND_ID;
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

}  // namespace omm
