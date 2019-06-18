#include "commands/pointstransformationcommand.h"
#include "objects/path.h"

namespace
{

auto get_old_points(const std::set<std::pair<omm::Path*, std::size_t>>& keys)
{
  omm::PointsTransformationCommand::Map map;
  for (auto&& [path, index] : keys) {
    const auto key = std::make_pair(path, index);
    map.insert(std::make_pair(key, path->points()[index]));
  }
  return map;
}

}  // namespace

namespace omm
{

PointsTransformationCommand::PointsTransformationCommand(const Map& new_points)
  : Command(QObject::tr("PointsTransformationCommand").toStdString())
  , m_old_points(get_old_points(::get_keys(new_points)))
  , m_new_points(new_points)
{
}

void PointsTransformationCommand::undo() { apply(m_old_points); }
void PointsTransformationCommand::redo() { apply(m_new_points); }

int PointsTransformationCommand::id() const
{
  return Command::POINTS_TRANSFORMATION_COMMAND_ID;
}

bool PointsTransformationCommand::mergeWith(const QUndoCommand* command)
{
  const auto& pt_command = static_cast<const PointsTransformationCommand&>(*command);
  const auto affected_points = this->affected_points();
  if (affected_points != pt_command.affected_points()) {
    return false;
  }

  for (const auto& key : affected_points) {
    m_new_points[key] = pt_command.m_new_points.at(key);
  }

  return true;
}

bool PointsTransformationCommand::is_noop() const
{
  for (const auto& key : affected_points()) {
    if (m_old_points.at(key) != m_new_points.at(key)) {
      return false;
    }
  }
  return true;
}

void PointsTransformationCommand::apply(const PointsTransformationCommand::Map &map)
{
  for (auto&& [k, v] : map) {
    Path* path = k.first;
    std::size_t index = k.second;
    path->point(index) = map.at(k);
  }
}

std::set<std::pair<Path*, std::size_t>> PointsTransformationCommand::affected_points() const
{
  const auto keys = ::get_keys(m_old_points);
  assert(keys == ::get_keys(m_new_points));
  return keys;
}

}  // namespace omm
