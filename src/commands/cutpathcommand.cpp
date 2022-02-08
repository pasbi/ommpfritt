#include "commands/cutpathcommand.h"
#include "commands/modifypointscommand.h"
#include "objects/pathobject.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"

namespace
{

using namespace omm;

std::deque<std::unique_ptr<PathPoint>> cut(PathPoint& a, PathPoint& b,
                                           const InterpolationMode interpolation,
                                           std::deque<double>&& positions,
                                           std::map<PathPoint*, Point>& modified_points)
{
  const auto control_points = Path::compute_control_points(a.geometry(), b.geometry(), interpolation);
  const auto geom_control_points = util::transform(control_points, std::mem_fn(&Vec2f::to_geom_point));
  const auto curve = std::unique_ptr<Geom::BezierCurve>(Geom::BezierCurve::create(geom_control_points));
  assert(std::is_sorted(positions.begin(), positions.end()));
  assert(!positions.empty());
  assert(positions.front() >= 0.0 && positions.back() <= 1.0);
  if (!positions.empty() && positions.front() == 0.0) {
    positions.pop_front();
  }
  if (!positions.empty() && positions.back() == 1.0) {
    positions.pop_back();
  }
  if (positions.empty()) {
    return {};
  }

  std::deque<std::unique_ptr<Geom::BezierCurve>> new_curves;
  new_curves.emplace_back(dynamic_cast<Geom::BezierCurve*>(curve->portion(0.0, positions.front())));
  for (std::size_t i = 0; i < positions.size() - 1; ++i) {
    new_curves.emplace_back(dynamic_cast<Geom::BezierCurve*>(curve->portion(positions[i], positions[i+1])));
  }
  new_curves.emplace_back(dynamic_cast<Geom::BezierCurve*>(curve->portion(positions.back(), 1.0)));

  Point left_point = a.geometry();
  left_point.set_right_position(Vec2f{new_curves.front()->controlPoint(1)});
  modified_points[&a] = left_point;

  Point right_point = b.geometry();
  right_point.set_left_position(Vec2f{new_curves.back()->controlPoint(2)});
  modified_points[&b] = right_point;

  std::deque<std::unique_ptr<PathPoint>> new_points;
  for (std::size_t i = 1; i < new_curves.size(); ++i) {
    // the last point of the previous curve must match the first point of the current one
    assert(new_curves[i-1]->controlPoint(3) == new_curves[i]->controlPoint(0));
    Point point{Vec2f{new_curves[i-1]->controlPoint(3)}};
    point.set_left_position(Vec2f{new_curves[i-1]->controlPoint(2)});
    point.set_right_position(Vec2f{new_curves[i]->controlPoint(1)});
    new_points.push_back(std::make_unique<PathPoint>(point, a.path()));
  }

  assert(new_points.size() ==  positions.size());
  return new_points;
}

void cut(Path& path,
         std::vector<Geom::PathTime>&& positions,
         const InterpolationMode interpolation,
         std::deque<omm::AddPointsCommand::OwnedLocatedPath>& new_point_sequences,
         std::map<PathPoint*, Point>& modified_points)
{
  assert(std::is_sorted(positions.begin(), positions.end()));

  std::map<std::size_t, std::deque<double>> curve_positions;
  for (const auto& position : positions) {
    curve_positions[position.curve_index].push_back(position.t);
  }

  for (auto&& [i, positions] : curve_positions) {
    const auto j = i == path.size() - 1 ? 0 : i + 1;
    auto new_points = cut(path.at(i), path.at(j), interpolation, std::move(positions), modified_points);
    if (!new_points.empty()) {
      new_point_sequences.emplace_back(&path, i + 1, std::move(new_points));
    }
  }
}

void cut(PathVector& path_vector,
         const InterpolationMode interpolation,
         const std::vector<Geom::PathVectorTime>& positions,
         std::deque<omm::AddPointsCommand::OwnedLocatedPath>& new_points,
         std::map<PathPoint*, Point>& modified_points)
{
  const auto paths = path_vector.paths();
  std::map<Path*, std::vector<Geom::PathTime>> path_positions;
  for (const auto position : positions) {
    path_positions[paths.at(position.path_index)].push_back(position.asPathTime());
  }

  for (auto&& [path, positions] : path_positions) {
    cut(*path, std::move(positions), interpolation, new_points, modified_points);
  }
}

}  // namespace

namespace omm
{

CutPathCommand::CutPathCommand(PathObject& path_object, const std::vector<Geom::PathVectorTime>& cuts)
    : CutPathCommand(QObject::tr("CutPathCommand"), path_object, cuts)
{
}

CutPathCommand::CutPathCommand(const QString& label,
                               PathObject& path_object,
                               const std::vector<Geom::PathVectorTime>& cuts)
    : ComposeCommand(label)
{
  const auto interpolation = path_object.property(PathObject::INTERPOLATION_PROPERTY_KEY)->value<InterpolationMode>();
  std::deque<omm::AddPointsCommand::OwnedLocatedPath> new_points;
  std::map<PathPoint*, Point> modified_points;
  cut(path_object.geometry(), interpolation, cuts, new_points, modified_points);
  std::vector<std::unique_ptr<Command>> commands;
  commands.push_back(std::make_unique<ModifyPointsCommand>(modified_points));
  commands.push_back(std::make_unique<AddPointsCommand>(path_object, std::move(new_points)));
  set_commands(std::move(commands));
}

}  // namespace omm
