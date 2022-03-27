#include "path/lib2geomadapter.h"
#include "path/path.h"
#include "path/pathvector.h"
#include "path/pathpoint.h"
#include <2geom/pathvector.h>

namespace omm
{

Geom::PathVector omm_to_geom(const PathVector& path_vector, InterpolationMode interpolation)
{
  Geom::PathVector paths;
  for (auto&& path : path_vector.paths()) {
    paths.push_back(omm_to_geom(*path, interpolation));
  }
  return paths;
}

Geom::Path omm_to_geom(const Path& path, InterpolationMode interpolation)
{
  std::vector<Geom::CubicBezier> bzs;
  const auto points = path.points();
  const std::size_t n = points.size();
  if (n == 0) {
    return Geom::Path{};
  }

  bzs.reserve(n - 1);

  std::unique_ptr<Path> smoothened;
  const Path* self = &path;
  if (interpolation == InterpolationMode::Smooth) {
    smoothened = std::make_unique<Path>(path);
    smoothened->smoothen();
    self = smoothened.get();
  }

  for (std::size_t i = 0; i < n - 1; ++i) {
    const auto cps = Path::compute_control_points(self->at(i).geometry(),
                                                  self->at(i + 1).geometry(),
                                                  interpolation);
    bzs.emplace_back(util::transform(cps, std::mem_fn(&Vec2f::to_geom_point)));
  }

  return {bzs.begin(), bzs.end()};
}

std::unique_ptr<PathVector> geom_to_omm(const Geom::PathVector& geom_path_vector)
{
  auto omm_path_vector = std::make_unique<PathVector>(nullptr);
  for (const auto& path : geom_path_vector) {
    omm_path_vector->add_path(geom_to_omm(path, omm_path_vector.get()));
  }
  return omm_path_vector;
}

void add_cubic_bezier_to_path(Path& omm_path, const Geom::CubicBezier& c)
{
  const auto p0 = Vec2f(c[0]);
  if (omm_path.size() == 0) {
    omm_path.add_point(Point{p0});
  }
  auto& last_point = *omm_path.points().back();
  auto geometry = last_point.geometry();
  geometry.set_right_tangent(PolarCoordinates(Vec2f(c[1]) - p0));
  last_point.set_geometry(geometry);
  const auto p1 = Vec2f(c[3]);
  auto& pref = omm_path.add_point(Point{p1});
  geometry = pref.geometry();
  geometry.set_left_tangent(PolarCoordinates(Vec2f(c[2]) - p1));
  pref.set_geometry(geometry);
}

std::unique_ptr<Path> geom_to_omm(const Geom::Path& geom_path, PathVector* parent)
{
  auto omm_path = std::make_unique<Path>(parent);
  const auto n = geom_path.size();
  for (std::size_t i = 0; i < n; ++i) {
    // non-bezier curves can occur, e.g., after boolean operations and can be ignored.
    if (const auto* c = dynamic_cast<const Geom::CubicBezier*>(&geom_path[i]); c != nullptr) {
      add_cubic_bezier_to_path(*omm_path, *c);
    }
  }

  return omm_path;
}

}  // namespace omm
