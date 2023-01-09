#include "path/lib2geomadapter.h"
#include "path/edge.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"

namespace omm
{

Geom::PathVector omm_to_geom(const PathVector& path_vector, const InterpolationMode interpolation)
{
  Geom::PathVector paths;
  for (auto&& path : path_vector.paths()) {
    paths.push_back(omm_to_geom(*path, interpolation));
  }
  assert(path_vector.paths().size() == paths.size());
  return paths;
}

template<InterpolationMode interp> GeomEdgeType<interp> omm_to_geom(const Edge& edge)
{
  const auto& a = edge.a()->geometry();
  const auto& b = edge.b()->geometry();
  const auto a_pos = a.position().to_geom_point();
  const auto b_pos = b.position().to_geom_point();

  if constexpr (interp == InterpolationMode::Linear) {
    return Geom::LineSegment(std::vector{a_pos, b_pos});
  } else {
    if (interp != InterpolationMode::Bezier) {
      LWARNING << "Smooth mode is not yet implemented.";
    }
    const auto a_t = a.tangent_position({edge.path(), Direction::Forward}).to_geom_point();
    const auto b_t = b.tangent_position({edge.path(), Direction::Backward}).to_geom_point();
    return Geom::BezierCurveN<3>(std::vector{a_pos, a_t, b_t, b_pos});
  }
}

Geom::Path omm_to_geom(const Path& path, const InterpolationMode interp)
{
  const auto make_path = [&path](const auto& edge_to_curve) {
    const auto curves = util::transform(path.edges(), edge_to_curve);
    return Geom::Path(curves.begin(), curves.end());
  };
  switch (interp) {
  case InterpolationMode::Bezier:
    return make_path([](const Edge* const edge) { return omm_to_geom<InterpolationMode::Bezier>(*edge); });
  case InterpolationMode::Linear:
    return make_path([](const Edge* const edge) { return omm_to_geom<InterpolationMode::Linear>(*edge); });
  case InterpolationMode::Smooth:
    return make_path([](const Edge* const edge) { return omm_to_geom<InterpolationMode::Smooth>(*edge); });
  }
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

template GeomEdgeType<InterpolationMode::Bezier> omm_to_geom<InterpolationMode::Bezier>(const Edge&);
template GeomEdgeType<InterpolationMode::Linear> omm_to_geom<InterpolationMode::Linear>(const Edge&);
template GeomEdgeType<InterpolationMode::Smooth> omm_to_geom<InterpolationMode::Smooth>(const Edge&);

}  // namespace omm
