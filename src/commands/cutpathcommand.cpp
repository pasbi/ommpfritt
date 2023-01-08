#include "commands/cutpathcommand.h"
#include "commands/addpointscommand.h"
#include "commands/addremovepointscommand.h"
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

struct Cutter
{
  explicit Cutter(const omm::PathVector& path_vector, const omm::InterpolationMode interpolation,
                  const std::vector<Geom::PathVectorTime>& positions)
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

    std::map<Index, std::set<double>, std::greater<>> xs;  // we need the cuts from end to start
    for (const auto& p : positions) {
      static constexpr auto eps = 0.0001;
      if (p.t > eps && p.t < 1.0 - eps) {
        xs[p].insert(p.t);
      }
    }

    for (const auto& [index, ts] : xs) {
      auto& path = *path_vector.paths().at(index.path_index);
      const auto make_point = [&path, this, &edge = *path.edges().at(index.edge_index), &path_vector](const double t) {
        const auto geom_edge = omm::omm_to_geom<omm::InterpolationMode::Bezier>(edge);
        using CurveType = decltype(geom_edge);
        const auto edge_before = std::unique_ptr<CurveType>(static_cast<CurveType*>(geom_edge.portion(0, t)));
        const auto edge_after = std::unique_ptr<CurveType>(static_cast<CurveType*>(geom_edge.portion(t, 1.0)));

        // a cut through an edge modifies the old end points (a and c) and creates a new point in between (b)
        auto a_geometry = edge.a()->geometry();
        auto b_geometry = omm::Point(omm::Vec2f(edge_before->finalPoint()));
        auto c_geometry = edge.b()->geometry();

        const auto fwd = omm::Point::TangentKey(&path, omm::Direction::Forward);
        const auto bwd = omm::Point::TangentKey(&path, omm::Direction::Backward);
        static constexpr auto to_polar = [](const auto& direction) {
          return omm::PolarCoordinates(omm::Vec2f(direction));
        };
        a_geometry.set_tangent(fwd, to_polar(edge_before->controlPoint(1) - edge_before->controlPoint(0)));
        b_geometry.set_tangent(bwd, to_polar(edge_before->controlPoint(2) - edge_before->controlPoint(3)));
        b_geometry.set_tangent(fwd, to_polar(edge_after->controlPoint(1) - edge_after->controlPoint(0)));
        c_geometry.set_tangent(bwd, to_polar(edge_after->controlPoint(2) - edge_after->controlPoint(3)));

        modified_points.try_emplace(edge.a().get(), a_geometry);
        modified_points.try_emplace(edge.b().get(), c_geometry);
        return std::make_shared<omm::PathPoint>(b_geometry, &path_vector);
      };
      auto new_points = util::transform<std::deque>(ts, make_point);
      LINFO << index.edge_index;
      new_path_segments.emplace_back(&path, index.edge_index + 1, std::move(new_points));
    }
  }

  std::deque<omm::OwnedLocatedPath> new_path_segments;
  std::map<omm::PathPoint*, omm::Point> modified_points;
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
  const auto interpolation = path_object.property(PathObject::INTERPOLATION_PROPERTY_KEY)->value<InterpolationMode>();
  ::Cutter cutter(path_object.path_vector(), interpolation, cuts);
  std::vector<std::unique_ptr<Command>> commands;
  commands.reserve(1 + cutter.new_path_segments.size());
  if (!cutter.modified_points.empty()) {
    commands.emplace_back(std::make_unique<ModifyPointsCommand>(cutter.modified_points));
  }
  for (auto& np : cutter.new_path_segments) {
    commands.emplace_back(std::make_unique<AddPointsCommand>(std::move(np), &path_object));
  }
  set_commands(std::move(commands));
}

}  // namespace omm
