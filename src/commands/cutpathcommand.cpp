#include "commands/cutpathcommand.h"
#include "objects/segment.h"
#include "commands/modifypointscommand.h"
#include "objects/path.h"

namespace
{

using namespace omm;

std::deque<std::unique_ptr<Point>> cut(Point& a, Point& b,
                                       std::deque<double>&& positions,
                                       InterpolationMode interpolation,
                                       std::map<Point*, Point>& modified_points)
{
  const auto control_points = Segment::compute_control_points(a, b, interpolation);
  const auto curve = std::unique_ptr<Geom::BezierCurve>(Geom::BezierCurve::create(control_points));
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

  Point left_point = a;
  left_point.set_right_position(Vec2f{new_curves.front()->controlPoint(1)});
  modified_points[&a] = left_point;

  Point right_point = b;
  right_point.set_left_position(Vec2f{new_curves.back()->controlPoint(2)});
  modified_points[&b] = right_point;

  std::deque<std::unique_ptr<Point>> new_points;
  for (std::size_t i = 1; i < new_curves.size(); ++i) {
    // the last point of the previous curve must match the first point of the current one
    assert(new_curves[i-1]->controlPoint(3) == new_curves[i]->controlPoint(0));
    auto point = std::make_unique<Point>(Vec2f{new_curves[i-1]->controlPoint(3)});
    point->set_left_position(Vec2f{new_curves[i-1]->controlPoint(2)});
    point->set_right_position(Vec2f{new_curves[i]->controlPoint(1)});
    new_points.push_back(std::move(point));
  }

  assert(new_points.size() ==  positions.size());
  return new_points;
}

void cut(Segment& segment,
         std::vector<Geom::PathTime>&& positions,
         const InterpolationMode interpolation,
         std::deque<omm::AddPointsCommand::OwnedLocatedSegment>& new_point_sequences,
         std::map<Point*, Point>& modified_points)
{
  assert(std::is_sorted(positions.begin(), positions.end()));

  std::map<std::size_t, std::deque<double>> curve_positions;
  for (const auto& position : positions) {
    curve_positions[position.curve_index].push_back(position.t);
  }

  for (auto&& [i, positions] : curve_positions) {
    const auto j = i == segment.size() - 1 ? 0 : i + 1;
    auto new_points = cut(segment.at(i), segment.at(j), std::move(positions), interpolation, modified_points);
    if (!new_points.empty()) {
      new_point_sequences.emplace_back(&segment, i + 1, std::move(new_points));
    }
  }
}

void cut(Path& path,
         const std::vector<Geom::PathVectorTime>& positions,
         std::deque<omm::AddPointsCommand::OwnedLocatedSegment>& new_points,
         ModifyPointsCommand::Map& modified_points)
{
  const auto segments = path.segments();
  std::map<Segment*, std::vector<Geom::PathTime>> path_positions;
  for (const auto position : positions) {
    path_positions[segments.at(position.path_index)].push_back(position.asPathTime());
  }

  const auto interpolation = path.property(Path::INTERPOLATION_PROPERTY_KEY)->value<InterpolationMode>();
  for (auto&& [segment, positions] : path_positions) {
    cut(*segment, std::move(positions), interpolation, new_points, modified_points[&path]);
  }
}

}  // namespace

namespace omm
{

CutPathCommand::CutPathCommand(Path& path, const std::vector<Geom::PathVectorTime>& cuts)
    : CutPathCommand(QObject::tr("CutPathCommand"), path, cuts)
{
}

CutPathCommand::CutPathCommand(const QString& label,
                               Path& path,
                               const std::vector<Geom::PathVectorTime>& cuts)
    : ComposeCommand(label)
{
  std::deque<omm::AddPointsCommand::OwnedLocatedSegment> new_points;
  ModifyPointsCommand::Map modified_points;
  cut(path, cuts, new_points, modified_points);
  std::vector<std::unique_ptr<Command>> commands;
  commands.push_back(std::make_unique<ModifyPointsCommand>(modified_points));
  commands.push_back(std::make_unique<AddPointsCommand>(path, std::move(new_points)));
  set_commands(std::move(commands));
}

}  // namespace omm
