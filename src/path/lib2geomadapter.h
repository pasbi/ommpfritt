#pragma once

#include "common.h"
#include <2geom/pathvector.h>

namespace omm
{

class PathVector;
class Path;
class Edge;

[[nodiscard]] Geom::PathVector omm_to_geom(const PathVector& path_vector,
                                           InterpolationMode interpolation = InterpolationMode::Bezier);
[[nodiscard]] Geom::Path omm_to_geom(const Path& path_vector,
                                     InterpolationMode interpolation = InterpolationMode::Bezier);
[[nodiscard]] std::unique_ptr<PathVector> geom_to_omm(const Geom::PathVector& path_vector);
[[nodiscard]] std::unique_ptr<Path> geom_to_omm(const Geom::Path& geom_path, PathVector* parent);

template<InterpolationMode interp> using GeomEdgeType =
    std::conditional_t<interp == InterpolationMode::Linear, Geom::LineSegment, Geom::BezierCurveN<3>>;
template<omm::InterpolationMode interp> [[nodiscard]] GeomEdgeType<interp> omm_to_geom(const omm::Edge& edge);

}  // namespace omm
