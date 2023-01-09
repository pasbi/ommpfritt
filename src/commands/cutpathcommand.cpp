#include "commands/cutpathcommand.h"
#include "commands/addpointscommand.h"
#include "commands/modifypointscommand.h"
#include "commands/ownedlocatedpath.h"
#include "objects/pathobject.h"
#include "path/edge.h"
#include "path/lib2geomadapter.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "transform.h"

namespace
{

struct Index
{
  Index(const Geom::PathVectorTime& pvt) : path_index(pvt.path_index), edge_index(pvt.curve_index)
  {
  }

  std::size_t path_index;
  std::size_t edge_index;

  [[nodiscard]] bool operator>(const Index& other) const noexcept
  {
    if (path_index == other.path_index) {
      return edge_index > other.edge_index;
    }
    return path_index > other.path_index;
  }
};

auto convert_cuts(const std::vector<Geom::PathVectorTime>& positions)
{
  std::map<Index, std::set<double>, std::greater<>> cuts;
  for (const auto& p : positions) {
    static constexpr auto eps = 0.0001;
    if (p.t > eps && p.t < 1.0 - eps) {
      cuts[p].insert(p.t);
    }
  }
  return cuts;
}

using CurveType = Geom::BezierCurveN<3>;

auto compute_portions(const omm::Edge& edge, std::set<double> ts)
{
  assert(!ts.empty());
  ts.insert(0.0);
  ts.insert(1.0);
  const auto ts_vec = std::vector(ts.begin(), ts.end());
  std::vector<std::unique_ptr<CurveType>> portions;
  portions.reserve(ts_vec.size() - 1);

  const auto curve = omm::omm_to_geom<omm::InterpolationMode::Bezier>(edge);

  for (std::size_t i = 1; i < ts_vec.size(); ++i) {
    // ownership of the curve portion is passed to the vector of unique_ptr.
    portions.emplace_back(static_cast<CurveType*>(curve.portion(ts_vec.at(i - 1), ts_vec.at(i))));
  }
  return portions;
}

class Cutter
{
public:
  Cutter(omm::Path& path, const std::size_t edge_index, std::set<double> ts,
         omm::ModifyPointsCommand::ModifiedPointsMap& modified_points)
    : m_path(path)
    , m_edge_index(edge_index)
    , m_edge(path.edge(edge_index))
    , m_portions(compute_portions(m_edge, std::move(ts)))
    , m_new_points(compute_new_points(compute_new_point_geometries()))
  {
    compute_end_point_modifications(modified_points);
  }

  [[nodiscard]] omm::OwnedLocatedPath&& new_points() noexcept
  {
    return std::move(m_new_points);
  }

private:
  [[nodiscard]] omm::OwnedLocatedPath compute_new_points(const std::deque<omm::Point>& geometries) const
  {
    auto points = util::transform(geometries, [this](const omm::Point& geometry) {
      return std::make_shared<omm::PathPoint>(geometry, m_path.path_vector());
    });

    return omm::OwnedLocatedPath(&m_path, m_edge_index + 1, std::move(points));
  }

  [[nodiscard]] std::deque<omm::Point> compute_new_point_geometries() const
  {
    std::deque<omm::Point> point_geometries;
    for (std::size_t i = 1; i < m_portions.size(); ++i) {
      const auto& curve = *m_portions.at(i);
      auto& geometry = point_geometries.emplace_back(omm::Vec2f(curve.initialPoint()));
      set_tangent(geometry, omm::Direction::Forward, curve);
      set_tangent(geometry, omm::Direction::Backward, *m_portions.at(i - 1));
    }
    return point_geometries;
  }

  void set_tangent(omm::Point& point, const omm::Direction direction, const CurveType& curve) const
  {
    const auto d = direction == omm::Direction::Forward ? curve.controlPoint(1) - curve.controlPoint(0)
                                                        : curve.controlPoint(2) - curve.controlPoint(3);
    point.set_tangent({&m_path, direction}, omm::PolarCoordinates(omm::Vec2f(d)));
  }

  void compute_end_point_modifications(omm::ModifyPointsCommand::ModifiedPointsMap& modified_points) const
  {
    const auto set_tangent = [&modified_points, this](omm::PathPoint& point, const omm::Direction direction,
                                                      const CurveType& curve) {
      auto& geometry = modified_points.try_emplace(&point, point.geometry()).first->second;
      this->set_tangent(geometry, direction, curve);
    };
    set_tangent(*m_edge.a(), omm::Direction::Forward, *m_portions.front());
    set_tangent(*m_edge.b(), omm::Direction::Backward, *m_portions.back());
  }

  omm::Path& m_path;
  const std::size_t m_edge_index;
  const omm::Edge& m_edge;
  const std::vector<std::unique_ptr<CurveType>> m_portions;
  omm::OwnedLocatedPath m_new_points;
};

}  // namespace

namespace omm
{

CutPathCommand::CutPathCommand(PathObject& path_object, const std::vector<Geom::PathVectorTime>& cuts)
  : CutPathCommand(QObject::tr("CutPathCommand"), path_object, cuts)
{
}

CutPathCommand::CutPathCommand(const QString& label, PathObject& path_object,
                               const std::vector<Geom::PathVectorTime>& cuts)
  : ComposeCommand(label)
{
  //  const auto interpolation = path_object.property(PathObject::INTERPOLATION_PROPERTY_KEY)->value<InterpolationMode>();
  const auto m_cuts = ::convert_cuts(cuts);

  omm::ModifyPointsCommand::ModifiedPointsMap modified_points;
  std::deque<omm::OwnedLocatedPath> new_path_segments;
  for (const auto& [index, ts] : m_cuts) {
    auto& path = path_object.path_vector().path(index.path_index);
    Cutter cutter(path, index.edge_index, ts, modified_points);
    new_path_segments.emplace_back(cutter.new_points());
  }

  std::vector<std::unique_ptr<Command>> commands;
  commands.reserve(1 + new_path_segments.size());
  if (!modified_points.empty()) {
    commands.emplace_back(std::make_unique<ModifyPointsCommand>(modified_points));
  }
  for (auto& np : new_path_segments) {
    commands.emplace_back(std::make_unique<AddPointsCommand>(std::move(np), &path_object));
  }
  set_commands(std::move(commands));
}

}  // namespace omm
