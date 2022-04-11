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
  (void) path;
  (void) interpolation;
  return Geom::Path{};




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
  (void) omm_path;
  (void) c;
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
