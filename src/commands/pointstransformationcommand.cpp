#include "commands/pointstransformationcommand.h"
#include "objects/path.h"

namespace
{
auto get_old_points(const std::set<omm::PathIterator>& keys)
{
  omm::PointsTransformationCommand::Map map;
  for (auto&& it : keys) {
    map.insert({it, *it});
  }
  return map;
}

}  // namespace

namespace omm
{
PointsTransformationCommand::PointsTransformationCommand(const Map& new_points)
    : Command(QObject::tr("PointsTransformationCommand")),
      m_old_points(get_old_points(::get_keys(new_points))), m_new_points(new_points)
{
}

void PointsTransformationCommand::undo()
{
  apply(m_old_points);
}
void PointsTransformationCommand::redo()
{
  apply(m_new_points);
}

int PointsTransformationCommand::id() const
{
  return Command::POINTS_TRANSFORMATION_COMMAND_ID;
}

bool PointsTransformationCommand::mergeWith(const QUndoCommand* command)
{
  const auto& pt_command = dynamic_cast<const PointsTransformationCommand&>(*command);
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
  const auto aps = this->affected_points();
  return std::any_of(aps.begin(), aps.end(), [this](const auto& key) {
    return m_old_points.at(key) != m_new_points.at(key);
  });
}

void PointsTransformationCommand::apply(const PointsTransformationCommand::Map& map)
{
  Q_UNUSED(map)
  std::set<Path*> paths;
  for (auto&& [k, v] : map) {
    *k = v;
    paths.insert(k.path);
  }
  for (auto&& path : paths) {
    path->update();
  }
}

std::set<PathIterator> PointsTransformationCommand::affected_points() const
{
  auto keys = ::get_keys(m_old_points);
  assert(keys == ::get_keys(m_new_points));
  return keys;
}

}  // namespace omm
