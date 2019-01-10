#include "commands/pointstransformationcommand.h"
#include "common.h"
#include "objects/object.h"
#include "objects/path.h"

namespace
{

auto
make_alternatives(const std::set<omm::Path*>& paths, const omm::ObjectTransformation& t)
{
  std::map<omm::Point*, omm::Point> alternatives;
  for (const auto& path : paths) {
    const auto pt = t.transformed(path->global_transformation());
    for (const auto& point : path->points()) {
      if (point->is_selected) {
        omm::Point other = pt.apply(*point);
        alternatives.insert(std::make_pair(point, other));
      }
    }
  }
  return alternatives;
}

template<typename MapT>
bool has_same_points(const MapT& a, const MapT& b)
{
  if (a.size() != b.size()) {
    return false;
  } else {
    for (const auto& p : a) {
      if (b.count(p.first) == 0) {
        return false;
      }
    }
    return true;
  }
}

}  // namespace

namespace omm
{

PointsTransformationCommand
::PointsTransformationCommand(const std::set<Path*>& paths, const ObjectTransformation& t)
  : Command("PointsTransformationCommand")
  , m_alternative_points(make_alternatives(paths, t))
  , m_paths(paths)
{
}

void PointsTransformationCommand::undo()
{
  redo();
}

void PointsTransformationCommand::redo()
{
  for (const auto& [point, _] : m_alternative_points) { point->swap(m_alternative_points[point]); }
  for (Path* path : m_paths) { path->update_interpolation(); }
}

int PointsTransformationCommand::id() const
{
  return POINTS_TRANSFORMATION_COMMAND_ID;
}

bool PointsTransformationCommand::mergeWith(const QUndoCommand* command)
{
  // merging happens automatically!
  const auto& ot_command = static_cast<const PointsTransformationCommand&>(*command);
  return has_same_points(ot_command.m_alternative_points, m_alternative_points);
}

}  // namespace omm
