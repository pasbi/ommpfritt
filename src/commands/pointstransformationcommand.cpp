#include "commands/pointstransformationcommand.h"
#include "common.h"
#include "objects/object.h"
#include "objects/path.h"

namespace
{

auto
make_alternatives(const std::set<omm::Path*>& paths, const omm::ObjectTransformation& t)
{
  std::map<omm::Path*, std::map<std::size_t, omm::Point>> alternatives;
  for (omm::Path* path : paths) {
    const auto pt = t.transformed(path->global_transformation());
    const auto points = path->points();
    for (std::size_t i = 0; i < points.size(); ++i) {
      if (points[i].is_selected) {
        omm::Point other = pt.apply(points[i]);
        alternatives[path].insert(std::make_pair(i, other));
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
  : Command(QObject::tr("PointsTransformationCommand").toStdString())
  , m_alternative_points(make_alternatives(paths, t))
{
}


void PointsTransformationCommand::redo()
{
  for (auto&& [path, alternatives] : m_alternative_points) {
    path->on_change(path, Path::POINTS_CHANGED, nullptr);
    const auto points = path->points_ref();
    for (const auto& [i, _] : alternatives) {
      points[i]->swap(alternatives[i]);
    }

    const auto& i_mode_property = path->property(Path::INTERPOLATION_PROPERTY_KEY);
    const auto i_mode = i_mode_property.value<Path::InterpolationMode>();
    for (auto [point, alternative] : path->modified_points(false, i_mode)) {
      point->swap(alternative);
    }
  }
}

void PointsTransformationCommand::undo() { redo(); }
int PointsTransformationCommand::id() const { return POINTS_TRANSFORMATION_COMMAND_ID; }

bool PointsTransformationCommand::mergeWith(const QUndoCommand* command)
{
  // merging happens automatically!
  const auto& ot_command = static_cast<const PointsTransformationCommand&>(*command);
  return has_same_points(ot_command.m_alternative_points, m_alternative_points);
}

}  // namespace omm
